#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

juce::AudioProcessorValueTreeState::ParameterLayout RiserLabProcessor::createLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout l;

    l.add (std::make_unique<AudioParameterChoice> (ParameterID { "length", 1 }, "Length",
            StringArray { "1/16", "1/8", "1/4", "1/2", "1 bar", "2 bars" }, 2));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "mix", 1 }, "Mix",
            NormalisableRange<float> (0.f, 1.f, 0.001f), 1.0f));
    l.add (std::make_unique<AudioParameterFloat> (ParameterID { "rise", 1 }, "Rise",
            NormalisableRange<float> (0.f, 1.f, 0.001f), 0.5f));
    l.add (std::make_unique<AudioParameterBool> (ParameterID { "trem", 1 }, "Tremolo", false));
    return l;
}

RiserLabProcessor::RiserLabProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

bool RiserLabProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainInputChannelSet()  == juce::AudioChannelSet::stereo()
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void RiserLabProcessor::prepareToPlay (double sampleRate, int)
{
    sr = sampleRate;
    bufSize = (int) (sr * 14.0);          // ~14 s: enough for 2 bars at 35 BPM
    ring.setSize (2, bufSize);
    ring.clear();
    writePos = 0;
    counter = 0;
    lastL = 0;
    startPos[0] = startPos[1] = 0;
    lpState[0] = lpState[1] = 0.f;
    tremPhase = 0.0;
}

void RiserLabProcessor::processBlock (juce::AudioBuffer<float>& buf, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int n = buf.getNumSamples();
    const int nch = juce::jmin (buf.getNumChannels(), 2);

    double bpm = 120.0;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm())
                bpm = *b;
    if (bpm < 20.0) bpm = 120.0;

    static const double beats[] = { 0.25, 0.5, 1.0, 2.0, 4.0, 8.0 };
    const int idx = juce::jlimit (0, 5, (int) apvts.getRawParameterValue ("length")->load());
    int L = (int) (beats[idx] * 60.0 / bpm * sr);
    L = juce::jlimit (64, bufSize / 2, L);
    L &= ~1;
    const int half = L / 2;

    if (L != lastL)
    {
        lastL = L;
        counter = 0;
        startPos[0] = startPos[1] = writePos;
    }

    const float mix  = apvts.getRawParameterValue ("mix")->load();
    const float rise = apvts.getRawParameterValue ("rise")->load();
    const bool  trem = apvts.getRawParameterValue ("trem")->load() > 0.5f;
    const float twoPi = juce::MathConstants<float>::twoPi;

    for (int i = 0; i < n; ++i)
    {
        const int p0 = (int) (counter % L);
        const int p1 = (int) ((counter + half) % L);

        float dry[2] = { 0.f, 0.f };
        for (int ch = 0; ch < nch; ++ch)
        {
            dry[ch] = buf.getReadPointer (ch)[i];
            ring.setSample (ch, writePos, dry[ch]);
        }

        if (p0 == 0) startPos[0] = writePos;
        if (p1 == 0) startPos[1] = writePos;

        const float s0 = std::sin (juce::MathConstants<float>::pi * (float) p0 / (float) L);
        const float s1 = std::sin (juce::MathConstants<float>::pi * (float) p1 / (float) L);
        const float w0 = s0 * s0, w1 = s1 * s1;

        const int r0 = (startPos[0] - p0 + bufSize) % bufSize;
        const int r1 = (startPos[1] - p1 + bufSize) % bufSize;

        const float g = (float) p0 / (float) L;                       // 0..1 inside the cycle
        const float cutoff = 30.f + rise * rise * 9000.f * g;         // high-pass sweep up = riser feel
        const float k = 1.f - std::exp (-twoPi * cutoff / (float) sr);

        float tremGain = 1.f;
        if (trem)
        {
            tremPhase += (double) twoPi * (2.0 + 14.0 * g) / sr;     // tremolo speeds up through the cycle
            if (tremPhase > twoPi) tremPhase -= twoPi;
            tremGain = 1.f - 0.6f * (0.5f + 0.5f * (float) std::sin (tremPhase));
        }

        for (int ch = 0; ch < nch; ++ch)
        {
            float wet = ring.getSample (ch, r0) * w0 + ring.getSample (ch, r1) * w1;
            lpState[ch] += k * (wet - lpState[ch]);
            wet = (wet - lpState[ch]) * tremGain;
            buf.getWritePointer (ch)[i] = dry[ch] * (1.f - mix) + wet * mix;
        }

        writePos = (writePos + 1) % bufSize;
        ++counter;
    }
}

juce::AudioProcessorEditor* RiserLabProcessor::createEditor()
{
    return new RiserLabEditor (*this);
}

void RiserLabProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void RiserLabProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RiserLabProcessor();
}

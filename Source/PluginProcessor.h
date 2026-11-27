#pragma once

#include <JuceHeader.h>
#include <vector>
#include <random>
#include "PluginEditor.h"

class BotanicalHarmonicGranularAudioProcessor : public juce::AudioProcessor
{
public:
    BotanicalHarmonicGranularAudioProcessor();
    ~BotanicalHarmonicGranularAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    void loadFile (const juce::File& file);
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
    const juce::AudioProcessorValueTreeState& getValueTreeState() const { return parameters; }

    friend class BotanicalHarmonicGranularAudioProcessorEditor;

    juce::Reverb reverb;
    juce::Reverb::Parameters reverbParams;

    juce::dsp::DelayLine<float> delayLineL; 
    juce::dsp::DelayLine<float> delayLineR;

    juce::AudioBuffer<float> circularBuffer;
    int circularBufferSize; 
    int circularWritePos = 0;

    // Grain class
    class Grain {
    public:
        Grain(int lengthinsamples, long long onset, int startpos, float rate, float amp, bool reverse)
            : lengthinsamples(lengthinsamples), onset(onset), startpos(startpos),
              rate(rate), amp(amp), reverse(reverse) {}

        float getEnvelope(int time, int envelopeType) const {
            int n = time - onset;
            if (n < 0 || n >= lengthinsamples) return 0.0f;

            const int fadeSamples = juce::jmin(5, lengthinsamples / 2);
            float fadeIn = n < fadeSamples ? n / (float)fadeSamples : 1.0f;
            float fadeOut = n >= lengthinsamples - fadeSamples ? 
                        (lengthinsamples - n) / (float)fadeSamples : 1.0f;

            float t = static_cast<float>(n) / (lengthinsamples - 1);
            float env = 1.0f;

            if (lengthinsamples <= 1) return 1.0f;
        
            switch (envelopeType)
            {
                case 0: // Hann
                    return 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * n / (lengthinsamples - 1)));
                case 1: // Hamming
                    return 0.54f - 0.46f * std::cos(juce::MathConstants<float>::twoPi * t);
                case 2: // Percussive (ramp down)
                    return 1.0f - t;
                case 3: // ExpDecay
                    return std::exp(-5.0f * t); // decay rate adjustable
                case 4: // RevExpDecay (attack)
                    return 1.0f - std::exp(-5.0f * (1.0f - t));
                case 5: // Cosine
                    return std::cos(juce::MathConstants<float>::halfPi * t); // fades out with cosine
                default:
                    return 1.0f;
            }

            return env * fadeIn * fadeOut;
        }

        int getSampleIndex(int time, int bufferSize) const {
            int n = time - onset;
            if (n < 0 || n >= lengthinsamples) return -1;
        
            int index = reverse ? startpos + (lengthinsamples - 1 - n)
                                : startpos + n;
            return index % bufferSize;
        }
        
        std::pair<float, float> getStereoSample(const juce::AudioBuffer<float>& buffer, int time, int bufferSize, int envelopeType) const {
            int index = getSampleIndex(time, bufferSize);
            if (index < 0 || index >= bufferSize)
                return { 0.0f, 0.0f };
        
            float env = getEnvelope(time, envelopeType); // <-- Now uses the envelope type
            float left = buffer.getNumChannels() > 0 ? buffer.getSample(0, index) : 0.0f;
            float right = buffer.getNumChannels() > 1 ? buffer.getSample(1, index) : left;
        
            return {
                left * env * amp,
                right * env * amp
            };
        }
        

        int lengthinsamples;
        long long onset;
        int startpos;
        float rate;
        float amp;
        bool reverse;
    };

    // Scheduler class 
    class Scheduler {
    public:
        Scheduler(float minIOI, float maxIOI)
            : minInteronset(minIOI), maxInteronset(maxIOI),
              rng(std::random_device{}()), dist(0.0f, 1.0f) {}

        float getNextInteronset() {
            return minInteronset + dist(rng) * (maxInteronset - minInteronset);
        }

    private:
        float minInteronset, maxInteronset;
        std::mt19937 rng;
        std::uniform_real_distribution<float> dist;
    };

private:
    juce::AudioFormatManager formatManager;
    juce::AudioBuffer<float> sampleBuffer;
    bool fileLoaded = false;

    // For glitch effects
    std::random_device rd;
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist{0.0f, 1.0f};
    int glitchCounter = 0;
    int glitchInterval = 0;
    float lastGlitchResolution = 0.0f;

    juce::LinearSmoothedValue<float> dryWetGain;
    juce::LinearSmoothedValue<float> grainGainCompensation;

    std::vector<Grain> activeGrains;
    Scheduler scheduler { 0.01f, 0.1f };
    bool getRandomBool() {
        std::bernoulli_distribution boolDist(0.5); // 50/50 chance
        return boolDist(rng);
    }

    void processGrains(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer, float grainWet);
    void processFX(juce::AudioBuffer<float>& buffer, float fxWet, bool freeze);

    int globalSampleCounter = 0;
    float timeUntilNextGrain = 0.0f;
    double currentSampleRate = 44100.0;

    int findZeroCrossing(int startPos, const juce::AudioBuffer<float>& buffer, bool reverse) {
        const int lookAhead = 100; // samples to look ahead/behind
        const int bufferSize = buffer.getNumSamples();
        const int numChannels = buffer.getNumChannels();
        
        if (reverse) {
            // Search backward for zero crossing
            for (int i = 0; i < lookAhead; ++i) {
                int pos = (startPos - i + bufferSize) % bufferSize;
                int nextPos = (pos - 1 + bufferSize) % bufferSize;
                
                float current = 0.0f;
                float next = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch) {
                    current += buffer.getSample(ch, pos);
                    next += buffer.getSample(ch, nextPos);
                }
                
                if (current * next <= 0.0f) { // Zero crossing found
                    return pos;
                }
            }
        } else {
            // Search forward for zero crossing
            for (int i = 0; i < lookAhead; ++i) {
                int pos = (startPos + i) % bufferSize;
                int nextPos = (pos + 1) % bufferSize;
                
                float current = 0.0f;
                float next = 0.0f;
                for (int ch = 0; ch < numChannels; ++ch) {
                    current += buffer.getSample(ch, pos);
                    next += buffer.getSample(ch, nextPos);
                }
                
                if (current * next <= 0.0f) { // Zero crossing found
                    return pos;
                }
            }
        }
        
        return startPos; // Return original if none found
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BotanicalHarmonicGranularAudioProcessor)
};

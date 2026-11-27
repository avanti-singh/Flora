// BotanicalHarmonicGranularAudioProcessor.cpp
#include "PluginProcessor.h"
#include "PluginEditor.h"

BotanicalHarmonicGranularAudioProcessor::BotanicalHarmonicGranularAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
#endif
    parameters(*this, nullptr, juce::Identifier("PARAMETERS"), 
        {
            std::make_unique<juce::AudioParameterFloat>("grain_size", "Grain Size", 1.0f, 1000.0f, 100.0f),
            std::make_unique<juce::AudioParameterFloat>("grain_density", "Grain Density", 1.0f, 100.0f, 50.0f),
            std::make_unique<juce::AudioParameterChoice>("grain_direction", "Grain Direction", 
                juce::StringArray({ "Forward", "Backward", "Both" }), 0),
            std::make_unique<juce::AudioParameterFloat>("dry_wet", "Dry/Wet", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterChoice>(
                "envelope_type", "Envelope Type",
                juce::StringArray({ "Hann", "Hamming", "Percussive", "Exp Decay", "Rev Exp Decay", "Cosine" }),
                0),
            std::make_unique<juce::AudioParameterBool>("freeze", "Freeze", false),
            std::make_unique<juce::AudioParameterFloat>("spray", "Spray", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("delay_wet", "Delay", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("reverb_wet", "Reverb", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("glitch_amt", "Glitch Amt", 0.0f, 1.0f, 0.0f),
            std::make_unique<juce::AudioParameterFloat>("glitch_rate", "Glitch Rate", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>("glitch_resolution", "Glitch Resolution", 0.0f, 1.0f, 0.5f)
        }),
        currentSampleRate(44100.0), // Initialize sample rate
        globalSampleCounter(0),
        timeUntilNextGrain(0.0f),
        circularBufferSize(0),
        circularWritePos(0),
        glitchCounter(0),
        glitchInterval(4410), // Default value
        lastGlitchResolution(0.5f)
{
    formatManager.registerBasicFormats();
}

BotanicalHarmonicGranularAudioProcessor::~BotanicalHarmonicGranularAudioProcessor() 
{    
    releaseResources();
}

const juce::String BotanicalHarmonicGranularAudioProcessor::getName() const { return "Flora"; }

bool BotanicalHarmonicGranularAudioProcessor::acceptsMidi() const { return false; }
bool BotanicalHarmonicGranularAudioProcessor::producesMidi() const { return false; }
bool BotanicalHarmonicGranularAudioProcessor::isMidiEffect() const { return false; }
double BotanicalHarmonicGranularAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int BotanicalHarmonicGranularAudioProcessor::getNumPrograms() { return 1; }
int BotanicalHarmonicGranularAudioProcessor::getCurrentProgram() { return 0; }
void BotanicalHarmonicGranularAudioProcessor::setCurrentProgram (int) {}
const juce::String BotanicalHarmonicGranularAudioProcessor::getProgramName (int) { return {}; }
void BotanicalHarmonicGranularAudioProcessor::changeProgramName (int, const juce::String&) {}

void BotanicalHarmonicGranularAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::ScopedNoDenormals noDenormals;

    // Validate sample rate and block size
    jassert(sampleRate > 0);
    jassert(samplesPerBlock > 0);
    
    // Store the sample rate and reset counters
    currentSampleRate = sampleRate;
    globalSampleCounter = 0;
    timeUntilNextGrain = 0.0f;

    // Circular buffer setup (2 seconds)
    circularBufferSize = static_cast<int>(sampleRate * 4.0); 
    // Sized for the wider of input and output: processBlock works on the full
    // output width (for example mono in, stereo out on a mono track).
    circularBuffer.setSize(juce::jmax(getTotalNumInputChannels(), getTotalNumOutputChannels()), circularBufferSize);
    circularBuffer.clear();
    circularWritePos = 0;
    activeGrains.clear();

    // Add these:
    dryWetGain.reset(sampleRate, 0.05);  // 50ms smoothing for dry/wet
    grainGainCompensation.reset(sampleRate, 0.02); // 20ms smoothing for grain gain

    // Reverb setup
    reverbParams.roomSize = 0.5f;
    reverbParams.damping = 0.5f;
    reverbParams.wetLevel = 1.0f;  // Controlled by parameter
    reverbParams.dryLevel = 1.0f;  // Controlled by parameter
    reverbParams.width = 1.0f;
    reverbParams.freezeMode = 0.0f;
    
    reverb.setSampleRate(sampleRate);
    reverb.setParameters(reverbParams);

    // Delay setup
    juce::dsp::ProcessSpec delaySpec;
    delaySpec.sampleRate = sampleRate;
    delaySpec.maximumBlockSize = samplesPerBlock;
    delaySpec.numChannels = 1;

    delayLineL.prepare(delaySpec);
    delayLineR.prepare(delaySpec);
    delayLineL.setMaximumDelayInSamples(sampleRate * 2); // 2 second max delay
    delayLineR.setMaximumDelayInSamples(sampleRate * 2);
    delayLineL.reset();
    delayLineR.reset();
    delayLineL.setDelay(0.5f * sampleRate); // 500ms default
    delayLineR.setDelay(0.5f * sampleRate); // 500ms default

    // Glitch effect initialization
    glitchCounter = 0;
    glitchInterval = static_cast<int>(sampleRate * 0.1f); // 100ms default
    lastGlitchResolution = 0.5f;

    // Random number generator seeding
    rng.seed(std::random_device{}());
    dist = std::uniform_real_distribution<float>(0.0f, 1.0f);

    // Reset any other DSP state that might need it
    scheduler = Scheduler(0.01f, 0.1f); // Reinitialize scheduler if needed
}

void BotanicalHarmonicGranularAudioProcessor::releaseResources() {}

void BotanicalHarmonicGranularAudioProcessor::processGrains(juce::AudioBuffer<float>& buffer, juce::AudioBuffer<float>& dryBuffer, float grainWet)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    // Get parameters safely
    auto grainSizeParam = parameters.getRawParameterValue("grain_size");
    auto densityParam = parameters.getRawParameterValue("grain_density");
    auto directionParam = parameters.getRawParameterValue("grain_direction");
    auto envelopeParam = parameters.getRawParameterValue("envelope_type");
    auto sprayParam = parameters.getRawParameterValue("spray");
    
    if (!grainSizeParam || !densityParam || !directionParam || !envelopeParam || !sprayParam) return;
    
    float grainSizeMs = grainSizeParam->load();
    float grainDensity = densityParam->load();
    int grainDirection = static_cast<int>(directionParam->load());
    int envelopeType = static_cast<int>(envelopeParam->load());
    float sprayAmount = sprayParam->load();

    int grainLengthSamples = juce::jmax(4, (int)(grainSizeMs / 1000.0f * currentSampleRate)); 

    // Update smoothed values at the start of processing
    dryWetGain.setTargetValue(grainWet);
    grainGainCompensation.setTargetValue(1.0f / std::sqrt(activeGrains.size() + 1));

    // Write to circular buffer
    for (int ch = 0; ch < numChannels; ch++) {
        const float* channelData = dryBuffer.getReadPointer(ch);
        for (int i = 0; i < numSamples; i++) {
            circularBuffer.setSample(ch, (circularWritePos + i) % circularBufferSize, channelData[i]);
        }
    }

    // Dynamic overlap calculation
    float minOverlap = 0.7f;
    float densityFactor = juce::jmap(grainDensity, 1.0f, 100.0f, 0.5f, 0.9f);
    float overlapFactor = minOverlap * densityFactor;

    // Process each sample
    for (int i = 0; i < numSamples; i++) {
        // Get the next smoothed values
        float currentDryWet = dryWetGain.getNextValue();
        float currentGainComp = grainGainCompensation.getNextValue();

        // Schedule new grains
        if (timeUntilNextGrain <= 0.0f) {
            int dir = (int)*parameters.getRawParameterValue("grain_direction");
            bool reverse = (dir == 1) || (dir == 2 && getRandomBool()); 
            
            // Find start position
            int startSample = circularWritePos;
            if (sprayAmount > 0.0f) {
                int maxLookback = std::min(circularBufferSize - grainLengthSamples, circularWritePos);
                int sprayRange = static_cast<int>(sprayAmount * maxLookback);
                int randomOffset = static_cast<int>(dist(rng) * (sprayRange + 1)); 
                startSample = (circularWritePos - randomOffset + circularBufferSize) % circularBufferSize;
            }

            // Zero-crossing detection
            startSample = findZeroCrossing(startSample, circularBuffer, reverse);
            startSample = juce::jmax(0, juce::jmin(startSample, circularBufferSize - grainLengthSamples));

            // Add new grain
            activeGrains.emplace_back(grainLengthSamples, globalSampleCounter, startSample, 1.0f, 1.0f, reverse);

            // Calculate next grain time with minimum spacing
            float secondsPerGrain = 1.0f / juce::jmax(1.0f, grainDensity);
            timeUntilNextGrain = juce::jmax(
                static_cast<float>(secondsPerGrain * currentSampleRate * 0.5f), // Minimum 50% spacing
                grainLengthSamples * overlapFactor
            );
        }
        timeUntilNextGrain -= 1.0f;

        // Mix active grains with smoothed gain
        float grainLeft = 0.0f;
        float grainRight = 0.0f;
        
        for (auto& grain : activeGrains) {
            auto [left, right] = grain.getStereoSample(circularBuffer, globalSampleCounter, circularBufferSize, envelopeType);
            grainLeft += left * currentGainComp;
            grainRight += right * currentGainComp;
        }

        // Dry/wet mix with smoothing
        float dryL = dryBuffer.getSample(0, i);
        float dryR = numChannels > 1 ? dryBuffer.getSample(1, i) : dryL;
        
        if (buffer.getNumChannels() > 0) {
            buffer.setSample(0, i, dryL * (1.0f - currentDryWet) + grainLeft * currentDryWet);
        }
        if (buffer.getNumChannels() > 1) {
            buffer.setSample(1, i, dryR * (1.0f - currentDryWet) + grainRight * currentDryWet);
        }

        globalSampleCounter++;
        
        // Update gain compensation if number of grains changed
        if (i % 8 == 0) { // Only check every 8 samples for performance
            grainGainCompensation.setTargetValue(1.0f / std::sqrt(activeGrains.size() + 1));
        }
    }

    // Cleanup finished grains
    activeGrains.erase(
        std::remove_if(activeGrains.begin(), activeGrains.end(),
            [this](const Grain& g) {
                return (globalSampleCounter - g.onset) >= g.lengthinsamples;
            }),
        activeGrains.end());

    circularWritePos = (circularWritePos + numSamples) % circularBufferSize;
}

void BotanicalHarmonicGranularAudioProcessor::processFX(juce::AudioBuffer<float>& buffer, float fxWet, bool freeze)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    
    // Get parameters
    const float delayWet = *parameters.getRawParameterValue("delay_wet");
    const float reverbWet = *parameters.getRawParameterValue("reverb_wet");
    const float glitchWet = *parameters.getRawParameterValue("glitch_amt");
    const float glitchRate = *parameters.getRawParameterValue("glitch_rate");
    const float glitchResolution = *parameters.getRawParameterValue("glitch_resolution");

    // Calculate glitch interval
    if (glitchResolution != lastGlitchResolution || glitchCounter >= glitchInterval) {
        float glitchFreq = juce::jmap(glitchRate, 0.0f, 1.0f, 1.0f, 100.0f);
        glitchInterval = static_cast<int>(currentSampleRate / glitchFreq);
        lastGlitchResolution = glitchResolution;
        glitchCounter = 0;
    }
    
    // Process glitch effect
    if (glitchWet > 0.0f) {
        static float heldSampleL = 0.0f;
        static float heldSampleR = 0.0f;

        for (int i = 0; i < numSamples; ++i) {
            glitchCounter = (glitchCounter + 1) % glitchInterval;

            if (glitchCounter < glitchInterval / 2) {
                for (int ch = 0; ch < numChannels; ++ch) {
                    float inSample = buffer.getSample(ch, i);

                    // Bit reduction
                    int bitDepth = static_cast<int>(juce::jmap(glitchResolution, 0.0f, 1.0f, 4.0f, 16.0f));
                    float maxVal = std::pow(2.0f, bitDepth - 1);
                    float quantized = std::round(inSample * maxVal) / maxVal;

                    // Sample rate reduction
                    int reduction = 1 + static_cast<int>(glitchResolution * 20);
                    if (i % reduction == 0) {
                        if (ch == 0) heldSampleL = quantized;
                        if (ch == 1) heldSampleR = quantized;
                    }

                    float held = (ch == 0 ? heldSampleL : heldSampleR);
                    float outSample = inSample * (1.0f - glitchWet) + held * glitchWet;
                    buffer.setSample(ch, i, outSample);
                }
            }
        }
    }
    
    // Process delay
    if (delayWet > 0.0f) {
        for (int i = 0; i < numSamples; ++i) {
            float inL = buffer.getSample(0, i);
            float inR = numChannels > 1 ? buffer.getSample(1, i) : inL;

            float delayedL = delayLineL.popSample(0);
            float delayedR = delayLineR.popSample(0);

            float inputWithFeedbackL = inL + delayedL * 0.5f;
            float inputWithFeedbackR = inR + delayedR * 0.5f;

            if (!freeze) {
                delayLineL.pushSample(0, inputWithFeedbackL);
                delayLineR.pushSample(0, inputWithFeedbackR);
            }

            buffer.setSample(0, i, inL * (1.0f - delayWet) + delayedL * delayWet);
            if (numChannels > 1) {
                buffer.setSample(1, i, inR * (1.0f - delayWet) + delayedR * delayWet);
            }
        }
    }

    
    // Process reverb
    if (reverbWet > 0.0f) {
        juce::AudioBuffer<float> reverbBuffer;
        reverbBuffer.makeCopyOf(buffer);
        
        // Process in blocks
        const int blockSize = 512;
        for (int pos = 0; pos < numSamples; pos += blockSize) {
            const int blockLength = juce::jmin(blockSize, numSamples - pos);
            
            if (numChannels == 2) {
                reverb.processStereo(reverbBuffer.getWritePointer(0, pos),
                                     reverbBuffer.getWritePointer(1, pos),
                                     blockLength);
            } else {
                reverb.processMono(reverbBuffer.getWritePointer(0, pos), blockLength);
                if (numChannels > 1) {
                    reverbBuffer.copyFrom(1, pos, reverbBuffer, 0, pos, blockLength);
                }
            }
        }
        
        // Mix reverb
        for (int ch = 0; ch < numChannels; ++ch) {
            buffer.addFrom(ch, 0, reverbBuffer, ch, 0, numSamples, reverbWet);
            buffer.applyGain(ch, 0, numSamples, 1.0f - reverbWet);
        }
    }
}

void BotanicalHarmonicGranularAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Safety checks
    if (buffer.getNumSamples() == 0 || buffer.getNumChannels() == 0) {
        return;
    }

    // Mono in, stereo out: feed the input to both sides.
    const int numIns = getTotalNumInputChannels();
    for (int ch = numIns; ch < buffer.getNumChannels(); ++ch) {
        if (numIns > 0)
            buffer.copyFrom(ch, 0, buffer, 0, 0, buffer.getNumSamples());
        else
            buffer.clear(ch, 0, buffer.getNumSamples());
    }

    if (circularBuffer.getNumChannels() < buffer.getNumChannels()) {
        return; // not prepared for this layout yet
    }

    // Get parameters
    float grainWet = *parameters.getRawParameterValue("dry_wet");
    bool freeze = *parameters.getRawParameterValue("freeze") > 0.5f;
    float fxWet = 1.0f; // Using full wet for FX chain
    
    // Preserve dry signal
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);
    
    // Process grains - writes to main buffer
    if (circularBuffer.getNumChannels() > 0 && circularBuffer.getNumSamples() > 0) {
        processGrains(buffer, dryBuffer, grainWet);
    }
    
    // Process FX on the granulated signal
    processFX(buffer, fxWet, freeze);
    
    // Final dry/wet mix if needed
    if (grainWet < 1.0f) {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
            buffer.addFrom(ch, 0, dryBuffer, ch, 0, buffer.getNumSamples(), 1.0f - grainWet);
        }
    }
}

juce::AudioProcessorEditor* BotanicalHarmonicGranularAudioProcessor::createEditor()
{
    return new BotanicalHarmonicGranularAudioProcessorEditor(*this);
}

bool BotanicalHarmonicGranularAudioProcessor::hasEditor() const { return true; }

void BotanicalHarmonicGranularAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BotanicalHarmonicGranularAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BotanicalHarmonicGranularAudioProcessor();
}
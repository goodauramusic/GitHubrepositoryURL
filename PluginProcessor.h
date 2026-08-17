#pragma once
#include <JuceHeader.h>
#include <vector>
#include <random>

class ChordFlowMelodyAudioProcessor : public juce::AudioProcessor
{
public:
    ChordFlowMelodyAudioProcessor();
    ~ChordFlowMelodyAudioProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override { return true; }

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "ChordFlow Melody"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return true; }
    bool isMidiEffect() const override { return true; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    void generateMelody();
    void setStyle(int style) { styleIndex = style; }
    void setComplexity(float v) { complexity = v; }
    void setDensity(float v) { density = v; }

private:
    struct Note { int pitch; int step; int length; float velocity; };

    std::vector<Note> melody;
    std::vector<int> activeChordNotes;
    std::vector<int> recentNotes;

    double currentSampleRate = 44100.0;
    int samplesUntilNextNote = 0;
    int generatedIndex = 0;
    int styleIndex = 0;
    float complexity = 0.55f;
    float density = 0.60f;
    int lastRoot = 60;
    int chordQuality = 0; // 0 maj7, 1 min7, 2 dom7, 3 min7b5

    std::mt19937 rng { std::random_device{}() };

    int chooseMelodyPitch(int root, int quality);
    void updateChord(const juce::MidiMessage& msg);
    void triggerGeneratedNote(juce::MidiBuffer& midi, int sampleOffset);
    bool isChordTone(int pitch, int root, int quality) const;
    int scalePitch(int root, int degree) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordFlowMelodyAudioProcessor)
};

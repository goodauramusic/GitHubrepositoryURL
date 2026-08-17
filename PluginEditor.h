#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ChordFlowMelodyAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit ChordFlowMelodyAudioProcessorEditor(ChordFlowMelodyAudioProcessor&);
    ~ChordFlowMelodyAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    ChordFlowMelodyAudioProcessor& processor;

    juce::Label title;
    juce::Label info;
    juce::ComboBox style;
    juce::Slider complexity;
    juce::Slider density;
    juce::TextButton generate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChordFlowMelodyAudioProcessorEditor)
};

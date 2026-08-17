#include "PluginEditor.h"

ChordFlowMelodyAudioProcessorEditor::ChordFlowMelodyAudioProcessorEditor(
    ChordFlowMelodyAudioProcessor& p) : AudioProcessorEditor(&p), processor(p)
{
    setSize(620, 380);

    title.setText("CHORDFLOW MELODY", juce::dontSendNotification);
    title.setFont(juce::Font(26.0f, juce::Font::bold));
    addAndMakeVisible(title);

    info.setText("Play a chord into the plugin. The generator follows the chord and outputs MIDI.",
                 juce::dontSendNotification);
    info.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(info);

    style.addItem("Pop", 1);
    style.addItem("Trap", 2);
    style.addItem("House", 3);
    style.addItem("R&B", 4);
    style.setSelectedId(1);
    style.onChange = [this] { processor.setStyle(style.getSelectedId() - 1); };
    addAndMakeVisible(style);

    complexity.setRange(0.0, 1.0, 0.01);
    complexity.setValue(0.55);
    complexity.setTextValueSuffix("  Complexity");
    complexity.onValueChange = [this] { processor.setComplexity((float)complexity.getValue()); };
    addAndMakeVisible(complexity);

    density.setRange(0.1, 1.0, 0.01);
    density.setValue(0.60);
    density.setTextValueSuffix("  Density");
    density.onValueChange = [this] { processor.setDensity((float)density.getValue()); };
    addAndMakeVisible(density);

    generate.setButtonText("GENERATE / REGENERATE");
    generate.onClick = [this] { processor.generateMelody(); };
    addAndMakeVisible(generate);
}

void ChordFlowMelodyAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds().reduced(12), 1);
}

void ChordFlowMelodyAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(24);
    title.setBounds(area.removeFromTop(45));
    info.setBounds(area.removeFromTop(45));

    style.setBounds(area.removeFromTop(42).reduced(0, 4));
    complexity.setBounds(area.removeFromTop(52).reduced(0, 5));
    density.setBounds(area.removeFromTop(52).reduced(0, 5));
    generate.setBounds(area.removeFromTop(52).reduced(0, 5));
}

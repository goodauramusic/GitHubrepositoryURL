#include "PluginProcessor.h"
#include "PluginEditor.h"

ChordFlowMelodyAudioProcessor::ChordFlowMelodyAudioProcessor()
    : AudioProcessor(BusesProperties()) {}

void ChordFlowMelodyAudioProcessor::prepareToPlay(double sr, int)
{
    currentSampleRate = sr;
    samplesUntilNextNote = 0;
}

void ChordFlowMelodyAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                                 juce::MidiBuffer& midi)
{
    buffer.clear();

    // Read incoming MIDI. Chords are inferred from simultaneously-held notes.
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        if (msg.isNoteOn() || msg.isNoteOff())
            updateChord(msg);
    }

    // Keep input MIDI and add generated MIDI. This makes the plugin useful
    // before a synth/instrument in a DAW.
    const int blockSize = buffer.getNumSamples();
    int offset = 0;

    while (offset < blockSize)
    {
        if (samplesUntilNextNote <= 0 && !melody.empty())
        {
            triggerGeneratedNote(midi, offset);
            const double bpm = 120.0;
            const double sixteenth = currentSampleRate * (60.0 / bpm) / 4.0;
            int rhythm = 1;
            if (styleIndex == 1) rhythm = (generatedIndex % 4 == 2 ? 2 : 1); // trap
            if (styleIndex == 2) rhythm = (generatedIndex % 4 == 1 ? 2 : 1); // house
            if (styleIndex == 3) rhythm = (generatedIndex % 4 == 3 ? 2 : 1); // R&B
            samplesUntilNextNote = juce::jmax(1, (int)(sixteenth * rhythm));
        }

        const int advance = juce::jmin(blockSize - offset, samplesUntilNextNote);
        samplesUntilNextNote -= advance;
        offset += advance;
    }
}

void ChordFlowMelodyAudioProcessor::updateChord(const juce::MidiMessage& msg)
{
    if (msg.isNoteOn())
    {
        const int n = msg.getNoteNumber();
        if (std::find(activeChordNotes.begin(), activeChordNotes.end(), n) == activeChordNotes.end())
            activeChordNotes.push_back(n);

        if (activeChordNotes.size() >= 2)
        {
            lastRoot = *std::min_element(activeChordNotes.begin(), activeChordNotes.end());
            chordQuality = 0;
            generateMelody();
        }
    }
    else if (msg.isNoteOff())
    {
        activeChordNotes.erase(
            std::remove(activeChordNotes.begin(), activeChordNotes.end(), msg.getNoteNumber()),
            activeChordNotes.end());
    }
}

bool ChordFlowMelodyAudioProcessor::isChordTone(int pitch, int root, int quality) const
{
    static const int maj7[]  = {0, 4, 7, 11};
    static const int min7[]  = {0, 3, 7, 10};
    static const int dom7[]  = {0, 4, 7, 10};
    static const int half[]  = {0, 3, 6, 10};

    const int* set = maj7;
    if (quality == 1) set = min7;
    if (quality == 2) set = dom7;
    if (quality == 3) set = half;

    const int pc = juce::jlimit(0, 11, (pitch - root) % 12 + 12) % 12;
    for (int i = 0; i < 4; ++i)
        if (pc == set[i]) return true;
    return false;
}

int ChordFlowMelodyAudioProcessor::scalePitch(int root, int degree) const
{
    static const int major[] = {0,2,4,5,7,9,11};
    static const int dorian[] = {0,2,3,5,7,9,10};
    static const int mixolydian[] = {0,2,4,5,7,9,10};

    const int* scale = major;
    if (styleIndex == 3) scale = dorian;
    if (styleIndex == 2) scale = mixolydian;

    const int oct = degree / 7;
    const int idx = degree % 7;
    return root + 12 * oct + scale[idx];
}

int ChordFlowMelodyAudioProcessor::chooseMelodyPitch(int root, int quality)
{
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
    std::uniform_int_distribution<int> degreeDist(0, 13);

    int best = root + 12 + degreeDist(rng) % 7;
    best = scalePitch(root, 7 + degreeDist(rng) % 7);

    // Strongly favour chord tones, but allow tasteful approach/passing notes.
    if (chance(rng) < (0.45f + 0.45f * complexity))
    {
        static const int chordOffsets[][4] = {
            {0,4,7,11}, {0,3,7,10}, {0,4,7,10}, {0,3,6,10}
        };
        const auto& set = chordOffsets[quality];
        best = root + 12 + set[std::uniform_int_distribution<int>(0,3)(rng)];
    }

    if (!recentNotes.empty() && chance(rng) < 0.55f)
    {
        const int prev = recentNotes.back();
        best = juce::jlimit(48, 84, prev + (std::uniform_int_distribution<int>(-4,4)(rng)));
    }

    return juce::jlimit(48, 84, best);
}

void ChordFlowMelodyAudioProcessor::generateMelody()
{
    melody.clear();
    recentNotes.clear();

    const int length = 16;
    std::uniform_real_distribution<float> chance(0.0f, 1.0f);

    for (int i = 0; i < length; ++i)
    {
        // Density creates rests, while the style adds rhythmic character later.
        if (chance(rng) > density && i % 4 != 0)
        {
            melody.push_back({-1, i, 1, 0.0f});
            continue;
        }

        int pitch = chooseMelodyPitch(lastRoot, chordQuality);

        // Every fourth step tends to resolve to a chord tone.
        if (i % 4 == 3)
        {
            static const int resolve[] = {0, 3, 7, 10};
            pitch = lastRoot + 12 + resolve[(i / 4 + styleIndex) % 4];
        }

        melody.push_back({pitch, i, 1, 0.70f + 0.20f * chance(rng)});
        recentNotes.push_back(pitch);
        if (recentNotes.size() > 4) recentNotes.erase(recentNotes.begin());
    }

    generatedIndex = 0;
}

void ChordFlowMelodyAudioProcessor::triggerGeneratedNote(juce::MidiBuffer& midi, int sampleOffset)
{
    if (melody.empty()) return;

    const auto note = melody[generatedIndex % melody.size()];
    ++generatedIndex;

    if (note.pitch < 0) return;

    midi.addEvent(juce::MidiMessage::noteOn(1, note.pitch,
                                            juce::uint8(juce::jlimit(1,127,(int)(note.velocity*127.0f)))),
                  sampleOffset);

    const int offSamples = juce::jmax(1, (int)(currentSampleRate * 0.08));
    midi.addEvent(juce::MidiMessage::noteOff(1, note.pitch), sampleOffset + offSamples);
}

void ChordFlowMelodyAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream out(destData, true);
    out.writeInt(styleIndex);
    out.writeFloat(complexity);
    out.writeFloat(density);
}

void ChordFlowMelodyAudioProcessor::setStateInformation(const void* data, int size)
{
    juce::MemoryInputStream in(data, (size_t)size, false);
    if (size >= 12)
    {
        styleIndex = in.readInt();
        complexity = in.readFloat();
        density = in.readFloat();
    }
}

juce::AudioProcessorEditor* ChordFlowMelodyAudioProcessor::createEditor()
{
    return new ChordFlowMelodyAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChordFlowMelodyAudioProcessor();
}

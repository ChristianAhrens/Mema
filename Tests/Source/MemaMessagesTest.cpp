/* Copyright (c) 2026, Christian Ahrens
 *
 * This file is part of Mema <https://github.com/ChristianAhrens/Mema>
 *
 * This tool is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This tool is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this tool; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

// Round-trip coverage for every Mema::SerializableMessage subclass declared in
// MemaMessages.h, in the same order they appear there. All exercise the exact
// production serialise/deserialise path (SerializableMessage::getSerializedMessage()
// / ::initFromMemoryBlock()), not a synthetic shortcut.

#include <JuceHeader.h>
#include <MemaProcessor/MemaMessages.h>

using namespace Mema;

//==============================================================================
class EnvironmentParametersMessageTest : public juce::UnitTest
{
public:
    EnvironmentParametersMessageTest() : juce::UnitTest ("EnvironmentParametersMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Palette style round-trips");

        EnvironmentParametersMessage original (JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Light);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        auto* epm = dynamic_cast<EnvironmentParametersMessage*> (deserialized);
        expect (epm != nullptr);
        if (epm != nullptr)
            expect (JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Light == epm->getPaletteStyle());

        SerializableMessage::freeMessageData (deserialized);
    }
};

static EnvironmentParametersMessageTest environmentParametersMessageTest;

//==============================================================================
class AnalyzerParametersMessageTest : public juce::UnitTest
{
public:
    AnalyzerParametersMessageTest() : juce::UnitTest ("AnalyzerParametersMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Sample rate and block size round-trip");

        AnalyzerParametersMessage original (48000, 512);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        expect (deserialized != nullptr, "initFromMemoryBlock() should recognise an AnalyzerParameters frame");

        auto* apm = dynamic_cast<AnalyzerParametersMessage*> (deserialized);
        expect (apm != nullptr, "initFromMemoryBlock() should construct an AnalyzerParametersMessage");

        if (apm != nullptr)
        {
            // NOTE: the wire payload is uint16, so values above 65535 (e.g. some high
            // sample rates) would already be truncated by the constructor itself, before
            // serialisation even happens -- not a round-trip bug, but worth knowing about
            // if this message is ever used with sample rates that don't fit uint16.
            expectEquals (apm->getSampleRate(), 48000);
            expectEquals (apm->getMaximumExpectedSamplesPerBlock(), 512);
        }

        SerializableMessage::freeMessageData (deserialized);
    }
};

static AnalyzerParametersMessageTest analyzerParametersMessageTest;

//==============================================================================
class ReinitIOCountMessageTest : public juce::UnitTest
{
public:
    ReinitIOCountMessageTest() : juce::UnitTest ("ReinitIOCountMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Input/output counts round-trip");

        ReinitIOCountMessage original (3, 12);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        expect (deserialized != nullptr, "initFromMemoryBlock() should recognise a ReinitIOCount frame");

        auto* riocm = dynamic_cast<ReinitIOCountMessage*> (deserialized);
        expect (riocm != nullptr, "initFromMemoryBlock() should construct a ReinitIOCountMessage");

        if (riocm != nullptr)
        {
            expectEquals (int (riocm->getInputCount()), 3);
            expectEquals (int (riocm->getOutputCount()), 12);
        }

        SerializableMessage::freeMessageData (deserialized);
    }
};

static ReinitIOCountMessageTest reinitIOCountMessageTest;

//==============================================================================
class AudioBufferMessageTest : public juce::UnitTest
{
public:
    AudioBufferMessageTest() : juce::UnitTest ("AudioInput/OutputBufferMessage round-trip", "Mema") {}

    void runTest() override
    {
        auto makeBuffer = [] (int numChannels, int numSamples, float firstSampleValue)
        {
            juce::AudioBuffer<float> buffer (numChannels, numSamples);
            auto v = firstSampleValue;
            for (int ch = 0; ch < numChannels; ++ch)
                for (int s = 0; s < numSamples; ++s)
                    buffer.setSample (ch, s, v++);
            return buffer;
        };

        beginTest ("AudioInputBufferMessage round-trips channel/sample data and flow direction");
        {
            auto buffer = makeBuffer (2, 6, 11.11f);
            AudioInputBufferMessage original (buffer);
            expect (AudioBufferMessage::FlowDirection::Input == original.getFlowDirection());

            auto blob = original.getSerializedMessage();
            auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
            auto* aibm = dynamic_cast<AudioInputBufferMessage*> (deserialized);
            expect (aibm != nullptr);

            if (aibm != nullptr)
            {
                expect (AudioBufferMessage::FlowDirection::Input == aibm->getFlowDirection());
                const auto& gotBuffer = aibm->getAudioBuffer();
                expectEquals (gotBuffer.getNumChannels(), 2);
                expectEquals (gotBuffer.getNumSamples(), 6);
                for (int ch = 0; ch < 2; ++ch)
                    for (int s = 0; s < 6; ++s)
                        expectEquals (gotBuffer.getSample (ch, s), buffer.getSample (ch, s));
            }

            SerializableMessage::freeMessageData (deserialized);
        }

        beginTest ("AudioOutputBufferMessage round-trips channel/sample data and flow direction");
        {
            auto buffer = makeBuffer (3, 4, -5.0f);
            AudioOutputBufferMessage original (buffer);
            expect (AudioBufferMessage::FlowDirection::Output == original.getFlowDirection());

            auto blob = original.getSerializedMessage();
            auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
            auto* aobm = dynamic_cast<AudioOutputBufferMessage*> (deserialized);
            expect (aobm != nullptr);

            if (aobm != nullptr)
            {
                const auto& gotBuffer = aobm->getAudioBuffer();
                expectEquals (gotBuffer.getNumChannels(), 3);
                expectEquals (gotBuffer.getNumSamples(), 4);
                for (int ch = 0; ch < 3; ++ch)
                    for (int s = 0; s < 4; ++s)
                        expectEquals (gotBuffer.getSample (ch, s), buffer.getSample (ch, s));
            }

            SerializableMessage::freeMessageData (deserialized);
        }
    }
};

static AudioBufferMessageTest audioBufferMessageTest;

//==============================================================================
class DataTrafficTypeSelectionMessageTest : public juce::UnitTest
{
public:
    DataTrafficTypeSelectionMessageTest() : juce::UnitTest ("DataTrafficTypeSelectionMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Traffic type list round-trips in order");

        std::vector<SerializableMessage::SerializableMessageType> types {
            SerializableMessage::ControlParameters,
            SerializableMessage::PluginParameterInfos,
            SerializableMessage::AudioOutputBuffer
        };
        DataTrafficTypeSelectionMessage original (types);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        auto* dttm = dynamic_cast<DataTrafficTypeSelectionMessage*> (deserialized);
        expect (dttm != nullptr);
        if (dttm != nullptr)
            expect (dttm->getTrafficTypes() == types);

        SerializableMessage::freeMessageData (deserialized);
    }
};

static DataTrafficTypeSelectionMessageTest dataTrafficTypeSelectionMessageTest;

//==============================================================================
// ControlParametersMessage -- the message used to exchange the full routing-matrix
// state (mutes, crosspoint enables, crosspoint gains) between Mema and Mema.Re. This
// gets its own, larger set of shapes because it's the most structurally complex
// message (nested maps, both a full-snapshot and several partial-update forms).
//
// NOTE: ControlParametersMessage::createSerializedContent() computes the wire
// element counts for the crosspoint sections as
// `m_crosspointStates.size() * m_crosspointStates.begin()->second.size()`
// (and the equivalent for crosspointValues) rather than summing each input's
// actual inner-map size. That is only correct when every input has the same
// number of entries -- true for every shape covered below, so these tests pass
// today. It is NOT true in general: a "ragged" map (inputs with differing
// output counts) would make the written count disagree with the number of
// (in, out, value) triples actually written, desynchronising every field read
// after it on the receiving end. See the accompanying report for a suggested
// fix; no call site in the current codebase constructs a ragged map, so no
// regression test for it is included here yet.
class ControlParametersMessageTest : public juce::UnitTest
{
public:
    ControlParametersMessageTest() : juce::UnitTest ("ControlParametersMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Empty message round-trips (pure mute-only updates send empty crosspoint maps)");
        {
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }

        beginTest ("Mute states round-trip");
        {
            std::map<std::uint16_t, bool> inputMuteStates { { 1, true }, { 2, false }, { 3, true } };
            std::map<std::uint16_t, bool> outputMuteStates { { 1, false }, { 4, true } };
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }

        beginTest ("Single crosspoint change round-trips (Mema's per-change echo to other clients)");
        {
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
            crosspointValues[2][5] = 0.75f;

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }

        beginTest ("Single input, several outputs round-trips (Mema.Re panning-mode update)");
        {
            // Mirrors PanningControlComponent::processOutputDistances(): one input channel
            // carries gains for several outputs at once, since the crossfade around a
            // hard-pan position can touch more than one output simultaneously.
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
            crosspointValues[3][1] = 0.1f;
            crosspointValues[3][2] = 0.9f;
            crosspointValues[3][6] = 0.3f;

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }

        beginTest ("Full dense NxM matrix snapshot round-trips (Mema's on-connect sync)");
        {
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
            for (std::uint16_t in = 1; in <= 4; ++in)
            {
                for (std::uint16_t out = 1; out <= 6; ++out)
                {
                    crosspointStates[in][out] = (in == out);
                    crosspointValues[in][out] = (in == out) ? 1.0f : 0.0f;
                }
            }

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }
    }

private:
    void roundTripAndCompare (const std::map<std::uint16_t, bool>& inputMuteStates,
                               const std::map<std::uint16_t, bool>& outputMuteStates,
                               const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates,
                               const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues)
    {
        ControlParametersMessage original (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        expect (deserialized != nullptr, "initFromMemoryBlock() should recognise a ControlParameters frame");

        auto* cpm = dynamic_cast<ControlParametersMessage*> (deserialized);
        expect (cpm != nullptr, "initFromMemoryBlock() should construct a ControlParametersMessage");

        if (cpm != nullptr)
        {
            expect (cpm->getInputMuteStates() == inputMuteStates, "input mute states should survive the round-trip");
            expect (cpm->getOutputMuteStates() == outputMuteStates, "output mute states should survive the round-trip");
            expect (cpm->getCrosspointStates() == crosspointStates, "crosspoint enable states should survive the round-trip");
            expect (cpm->getCrosspointValues() == crosspointValues, "crosspoint gain values should survive the round-trip");
        }

        SerializableMessage::freeMessageData (deserialized);
    }
};

static ControlParametersMessageTest controlParametersMessageTest;

//==============================================================================
class PluginParameterInfosMessageTest : public juce::UnitTest
{
public:
    PluginParameterInfosMessageTest() : juce::UnitTest ("PluginParameterInfosMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Plugin name, state, and several parameters (with and without step names) round-trip");

        PluginParameterInfo continuous;
        continuous.index = 0;
        continuous.id = "gain";
        continuous.name = "Gain";
        continuous.defaultValue = 0.5f;
        continuous.currentValue = 0.75f;
        continuous.label = "dB";
        continuous.isAutomatable = true;
        continuous.isRemoteControllable = true;
        continuous.category = juce::AudioProcessorParameter::genericParameter;
        continuous.minValue = 0.0f;
        continuous.maxValue = 1.0f;
        continuous.stepSize = 0.0f;
        continuous.isDiscrete = false;
        continuous.type = ParameterControlType::Continuous;
        continuous.stepCount = 0;

        PluginParameterInfo discrete;
        discrete.index = 1;
        discrete.id = "mode";
        discrete.name = juce::CharPointer_UTF8 ("Mod\xc3\xa9"); // "Modé" -- exercise UTF-8 handling
        discrete.defaultValue = 0.0f;
        discrete.currentValue = 1.0f;
        discrete.label = "";
        discrete.isAutomatable = false;
        discrete.isRemoteControllable = false;
        discrete.category = juce::AudioProcessorParameter::genericParameter;
        discrete.minValue = 0.0f;
        discrete.maxValue = 2.0f;
        discrete.stepSize = 1.0f;
        discrete.isDiscrete = true;
        discrete.type = ParameterControlType::Discrete;
        discrete.stepCount = 3;
        discrete.stepNames = { "Low", "Mid", "High" };

        std::vector<PluginParameterInfo> infos { continuous, discrete };

        PluginParameterInfosMessage original ("My Plugin", true, false, infos);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        auto* ppim = dynamic_cast<PluginParameterInfosMessage*> (deserialized);
        expect (ppim != nullptr);

        if (ppim != nullptr)
        {
            expectEquals (ppim->getPluginName(), juce::String ("My Plugin"));
            expect (ppim->isPluginEnabled());
            expect (! ppim->isPluginPost());

            auto& gotInfos = ppim->getParameterInfos();
            expectEquals ((int) gotInfos.size(), 2);
            expect (gotInfos[0] == continuous, "continuous parameter should round-trip exactly");
            // NOTE: PluginParameterInfo::operator== deliberately/accidentally does not compare
            // stepNames (only stepCount), so this alone would pass even with garbled step name
            // content -- check the actual UTF-8 step name text explicitly too.
            expect (gotInfos[1] == discrete, "discrete parameter (incl. UTF-8 name) should round-trip exactly");
            expectEquals (gotInfos[1].name, discrete.name, "UTF-8 parameter name should survive the round-trip");
            expect (gotInfos[1].stepNames == discrete.stepNames, "step name list content should survive the round-trip");
        }

        SerializableMessage::freeMessageData (deserialized);
    }
};

static PluginParameterInfosMessageTest pluginParameterInfosMessageTest;

//==============================================================================
class PluginParameterValueMessageTest : public juce::UnitTest
{
public:
    PluginParameterValueMessageTest() : juce::UnitTest ("PluginParameterValueMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Parameter index, id, and value round-trip");

        PluginParameterValueMessage original (7, "cutoffFreq", 0.42f);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        auto* ppvm = dynamic_cast<PluginParameterValueMessage*> (deserialized);
        expect (ppvm != nullptr);
        if (ppvm != nullptr)
        {
            expectEquals ((int) ppvm->getParameterIndex(), 7);
            expectEquals (ppvm->getParameterId(), juce::String ("cutoffFreq"));
            expectEquals (ppvm->getCurrentValue(), 0.42f);
        }

        SerializableMessage::freeMessageData (deserialized);
    }
};

static PluginParameterValueMessageTest pluginParameterValueMessageTest;

//==============================================================================
class PluginProcessingStateMessageTest : public juce::UnitTest
{
public:
    PluginProcessingStateMessageTest() : juce::UnitTest ("PluginProcessingStateMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Enabled and post-matrix flags round-trip (true/false combinations)");

        for (bool enabled : { true, false })
        {
            for (bool post : { true, false })
            {
                PluginProcessingStateMessage original (enabled, post);
                auto blob = original.getSerializedMessage();

                auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
                auto* ppsm = dynamic_cast<PluginProcessingStateMessage*> (deserialized);
                expect (ppsm != nullptr);
                if (ppsm != nullptr)
                {
                    expect (ppsm->isEnabled() == enabled);
                    expect (ppsm->isPost() == post);
                }

                SerializableMessage::freeMessageData (deserialized);
            }
        }
    }
};

static PluginProcessingStateMessageTest pluginProcessingStateMessageTest;

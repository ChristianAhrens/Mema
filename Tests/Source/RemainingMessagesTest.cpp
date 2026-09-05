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

// Round-trip coverage for the Mema::SerializableMessage subclasses not already
// covered by ControlParametersMessageTest.cpp / SimpleMessagesTest.cpp:
// AudioInputBufferMessage, AudioOutputBufferMessage, DataTrafficTypeSelectionMessage,
// EnvironmentParametersMessage, PluginParameterInfosMessage, PluginParameterValueMessage,
// and PluginProcessingStateMessage. All exercise the real production path
// (SerializableMessage::getSerializedMessage() / ::initFromMemoryBlock()).

#include <JuceHeader.h>
#include <MemaProcessor/MemaMessages.h>

using namespace Mema;

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
                    expectEquals (ppsm->isEnabled(), enabled);
                    expectEquals (ppsm->isPost(), post);
                }

                SerializableMessage::freeMessageData (deserialized);
            }
        }
    }
};

static PluginProcessingStateMessageTest pluginProcessingStateMessageTest;

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

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
// Header mechanics shared by every message type: the fixed {protocolVersion, messageType}
// wire header (SerializableMessage::getWireHeaderSize()/peekType()), and initFromMemoryBlock()'s
// rejection of a version mismatch or a too-short frame. The per-message tests below already
// exercise this header implicitly on every round-trip; this test targets the header mechanics
// directly and in isolation.
class SerializableMessageHeaderTest : public juce::UnitTest
{
public:
    SerializableMessageHeaderTest() : juce::UnitTest ("SerializableMessage header mechanics", "Mema") {}

    void runTest() override
    {
        beginTest ("getWireHeaderSize() is exactly two uint16_t fields");
        expectEquals ((int) SerializableMessage::getWireHeaderSize(), 4);

        beginTest ("Serialised frame starts with the current protocol version, then the correct type");
        {
            ReinitIOCountMessage original (3, 12);
            auto blob = original.getSerializedMessage();

            expect (blob.getSize() >= SerializableMessage::getWireHeaderSize());

            std::uint16_t version = 0;
            blob.copyTo (&version, 0, sizeof (std::uint16_t));
            expectEquals ((int) version, (int) SerializableMessage::s_protocolVersion);

            expect (SerializableMessage::ReinitIOCount == SerializableMessage::peekType (blob),
                    "peekType() should read the type from offset 2, not offset 0");
        }

        beginTest ("A frame with a mismatched protocol version is rejected, not parsed");
        {
            ReinitIOCountMessage original (3, 12);
            auto blob = original.getSerializedMessage();

            std::uint16_t wrongVersion = (std::uint16_t) (SerializableMessage::s_protocolVersion + 1);
            blob.copyFrom (&wrongVersion, 0, sizeof (std::uint16_t));

            auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
            expect (deserialized == nullptr, "a version mismatch must yield nullptr, not a misparsed message");
            SerializableMessage::freeMessageData (deserialized);
        }

        beginTest ("A frame shorter than the header is rejected, not parsed");
        {
            juce::MemoryBlock tooShort;
            std::uint16_t version = SerializableMessage::s_protocolVersion;
            tooShort.append (&version, sizeof (std::uint16_t)); // header is 4 bytes; this is only 2

            auto* deserialized = SerializableMessage::initFromMemoryBlock (tooShort);
            expect (deserialized == nullptr, "a frame shorter than getWireHeaderSize() must yield nullptr");
            SerializableMessage::freeMessageData (deserialized);

            auto* fromEmpty = SerializableMessage::initFromMemoryBlock (juce::MemoryBlock());
            expect (fromEmpty == nullptr, "an empty frame must yield nullptr");
            SerializableMessage::freeMessageData (fromEmpty);
        }

        beginTest ("Every message type's serialised header matches its own type enum value");
        {
            juce::AudioBuffer<float> emptyBuffer (1, 1);

            expectHeaderType (EnvironmentParametersMessage (JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Dark),
                               SerializableMessage::EnvironmentParameters);
            expectHeaderType (AnalyzerParametersMessage (48000, 512), SerializableMessage::AnalyzerParameters);
            expectHeaderType (ReinitIOCountMessage (2, 2), SerializableMessage::ReinitIOCount);
            expectHeaderType (AudioInputBufferMessage (emptyBuffer), SerializableMessage::AudioInputBuffer);
            expectHeaderType (AudioOutputBufferMessage (emptyBuffer), SerializableMessage::AudioOutputBuffer);

            // Named, not a bare {} literal: DataTrafficTypeSelectionMessage's (const
            // std::vector<...>&) and (const juce::MemoryBlock&) constructors are both
            // viable from a braced-init-list argument (juce::MemoryBlock has a (size_t,
            // bool) constructor reachable via implicit enum conversions), so passing {}
            // directly is ambiguous -- a compile error, not a runtime concern.
            std::vector<SerializableMessage::SerializableMessageType> noTrafficTypes;
            expectHeaderType (DataTrafficTypeSelectionMessage (noTrafficTypes), SerializableMessage::DataTrafficTypeSelection);
            expectHeaderType (ControlParametersMessage ({}, {}, {}, {}), SerializableMessage::ControlParameters);
            expectHeaderType (PluginParameterInfosMessage ("", false, false, {}), SerializableMessage::PluginParameterInfos);
            expectHeaderType (PluginParameterValueMessage (0, "x", 0.0f), SerializableMessage::PluginParameterValue);
            expectHeaderType (PluginProcessingStateMessage (true, false), SerializableMessage::PluginProcessingState);
        }
    }

private:
    template <typename MessageType>
    void expectHeaderType (const MessageType& message, SerializableMessage::SerializableMessageType expectedType)
    {
        auto blob = message.getSerializedMessage();
        expect (expectedType == SerializableMessage::peekType (blob));
    }
};

static SerializableMessageHeaderTest serializableMessageHeaderTest;

//==============================================================================
// Malformed/truncated-frame rejection, for every message type: a frame that fails to parse
// must yield nullptr from initFromMemoryBlock() -- never a crash, and never a half-parsed
// object -- whether the frame is too short (no payload at all, or one byte short of complete)
// or one of its own length-prefixed counts claims more data than was actually sent. Every
// constructor's payload parsing goes through FrameReader (see MemaMessages.h); this is the
// test suite for the guarantee that migration was meant to provide.
class MalformedFrameRejectionTest : public juce::UnitTest
{
public:
    MalformedFrameRejectionTest() : juce::UnitTest ("Malformed/truncated frame rejection", "Mema") {}

    void runTest() override
    {
        juce::AudioBuffer<float> buffer (2, 4);
        for (int ch = 0; ch < 2; ++ch)
            for (int s = 0; s < 4; ++s)
                buffer.setSample (ch, s, 1.0f);

        PluginParameterInfo oneParamInfo;
        oneParamInfo.index = 0;
        oneParamInfo.id = "x";
        oneParamInfo.name = "X";
        oneParamInfo.label = "";
        oneParamInfo.category = juce::AudioProcessorParameter::genericParameter;
        oneParamInfo.type = ParameterControlType::Continuous;
        std::vector<PluginParameterInfo> oneParam { oneParamInfo };

        beginTest ("Header-only and one-byte-short truncations are rejected for every message type");
        {
            expectTruncationsRejected (EnvironmentParametersMessage (JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Dark));
            expectTruncationsRejected (AnalyzerParametersMessage (48000, 512));
            expectTruncationsRejected (ReinitIOCountMessage (2, 2));
            expectTruncationsRejected (AudioInputBufferMessage (buffer));
            expectTruncationsRejected (AudioOutputBufferMessage (buffer));
            std::vector<SerializableMessage::SerializableMessageType> someTrafficTypes {
                SerializableMessage::ControlParameters, SerializableMessage::AudioOutputBuffer };
            expectTruncationsRejected (DataTrafficTypeSelectionMessage (someTrafficTypes));

            std::map<std::uint16_t, bool> oneMuteState { { 1, true } };
            expectTruncationsRejected (ControlParametersMessage (
                oneMuteState, std::map<std::uint16_t, bool>(),
                std::map<std::uint16_t, std::map<std::uint16_t, bool>>(),
                std::map<std::uint16_t, std::map<std::uint16_t, float>>()));

            expectTruncationsRejected (PluginParameterInfosMessage ("P", true, false, oneParam));
            expectTruncationsRejected (PluginParameterValueMessage (0, "x", 0.5f));
            expectTruncationsRejected (PluginProcessingStateMessage (true, false));
        }

        beginTest ("A count field claiming more entries than were actually sent is rejected, not read out of bounds");
        {
            // Build a well-formed PluginParameterInfosMessage (one parameter), then overwrite
            // its paramCount field with a value far larger than what was actually serialised.
            // Wire layout after the header: pluginNameLength(2) + "P"(1) + enabled(1) + post(1)
            // + paramCount(2) + ... -- see PluginParameterInfosMessage::createSerializedContent().
            PluginParameterInfosMessage original ("P", true, false, oneParam);
            auto blob = original.getSerializedMessage();

            auto paramCountOffset = SerializableMessage::getWireHeaderSize()
                + sizeof (std::uint16_t) /* pluginNameLength field */
                + 1 /* "P" */ + 1 /* enabled */ + 1 /* post */;
            std::uint16_t bogusCount = 60000;
            blob.copyFrom (&bogusCount, (int) paramCountOffset, sizeof (std::uint16_t));

            auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
            expect (deserialized == nullptr, "a paramCount far exceeding the actual payload must be rejected, not crash");
            SerializableMessage::freeMessageData (deserialized);
        }

        beginTest ("Channel/sample counts large enough to overflow a 32-bit size_t multiplication are still rejected safely");
        {
            // Regression test for the uint64_t-explicit size calculation in
            // AudioInputBufferMessage/AudioOutputBufferMessage's MemoryBlock constructor: two
            // wire-supplied uint16_t counts multiplied as a 32-bit size_t (this project ships
            // 32-bit RaspberryPiOS builds) can wrap around to a small value and defeat the
            // bounds check it was meant to enforce.
            juce::AudioBuffer<float> smallBuffer (1, 1);
            smallBuffer.setSample (0, 0, 1.0f);
            AudioInputBufferMessage original (smallBuffer);
            auto blob = original.getSerializedMessage();

            auto numChannelsOffset = SerializableMessage::getWireHeaderSize() + sizeof (AudioBufferMessage::FlowDirection);
            std::uint16_t hugeCount = 60000; // 60000 * 60000 * 4 bytes ~= 14.4 GB
            blob.copyFrom (&hugeCount, (int) numChannelsOffset, sizeof (std::uint16_t));
            blob.copyFrom (&hugeCount, (int) (numChannelsOffset + sizeof (std::uint16_t)), sizeof (std::uint16_t));

            auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
            expect (deserialized == nullptr, "a channel/sample count claiming far more data than was sent must be rejected, not wrap around and read out of bounds");
            SerializableMessage::freeMessageData (deserialized);
        }
    }

private:
    template <typename MessageType>
    void expectTruncationsRejected (const MessageType& message)
    {
        auto blob = message.getSerializedMessage();
        auto headerSize = SerializableMessage::getWireHeaderSize();

        {
            juce::MemoryBlock headerOnly (blob.getData(), headerSize);
            auto* deserialized = SerializableMessage::initFromMemoryBlock (headerOnly);
            expect (deserialized == nullptr, "a header-only frame (no payload at all) must be rejected");
            SerializableMessage::freeMessageData (deserialized);
        }

        if (blob.getSize() > headerSize)
        {
            juce::MemoryBlock almostComplete (blob.getData(), blob.getSize() - 1);
            auto* deserialized = SerializableMessage::initFromMemoryBlock (almostComplete);
            expect (deserialized == nullptr, "a frame one byte short of complete must be rejected");
            SerializableMessage::freeMessageData (deserialized);
        }
    }
};

static MalformedFrameRejectionTest malformedFrameRejectionTest;

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
// message (nested maps, both a full-snapshot and several partial-update forms) and
// carries a regression test for a real bug found and fixed while writing these
// tests (see the NOTE below).
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

        beginTest ("Ragged map (inputs with differing output counts) round-trips [regression]");
        {
            // Regression test for a fixed createSerializedContent() bug: it used to compute
            // the crosspoint sections' wire element counts as
            // `m_crosspointStates.size() * m_crosspointStates.begin()->second.size()`
            // (and the equivalent for crosspointValues) instead of summing each input's
            // actual inner-map size. That was only correct when every input had the same
            // number of entries, which happened to be true for every shape any call site
            // constructed at the time -- but a genuinely ragged map (three inputs, each with
            // a different number of outputs) made the written count disagree with the number
            // of (in, out, value) triples actually written, desynchronising every field read
            // after it on the receiving end. Fixed in createSerializedContent() to sum actual
            // per-input sizes; this test guards against a regression.
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
            crosspointStates[1][1] = true;
            crosspointStates[2][1] = true;
            crosspointStates[2][2] = false;
            crosspointStates[3][1] = true;
            crosspointStates[3][2] = true;
            crosspointStates[3][3] = true;
            crosspointValues[1][1] = 1.0f;
            crosspointValues[2][1] = 0.5f;
            crosspointValues[2][2] = 0.25f;
            crosspointValues[3][1] = 0.1f;
            crosspointValues[3][2] = 0.2f;
            crosspointValues[3][3] = 0.3f;

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

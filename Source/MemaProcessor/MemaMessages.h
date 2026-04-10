/* Copyright (c) 2024-2025, Christian Ahrens
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

#pragma once

#include <JuceHeader.h>

#include <CustomLookAndFeel.h>

#include "MemaPluginParameterInfo.h"

namespace Mema
{


//==============================================================================
/*
 * Forward declarations of SerializableMessage-derived specializations
 */
class EnvironmentParametersMessage;
class AnalyzerParametersMessage;
class ReinitIOCountMessage;
class AudioInputBufferMessage;
class AudioOutputBufferMessage;
class DataTrafficTypeSelectionMessage;
class ControlParametersMessage;
class PluginParameterInfosMessage;
class PluginParameterValueMessage;
class PluginProcessingStateMessage;

/**
 * @class SerializableMessage
 * @brief Base class for all messages exchanged between Mema, Mema.Mo, and Mema.Re over TCP.
 *
 * @details
 * All inter-process communication between the Mema server and its clients (Mema.Mo, Mema.Re)
 * uses a simple binary framing protocol built on top of JUCE's `InterprocessConnection`:
 *
 * | Offset | Size | Content |
 * |--------|------|---------|
 * | 0      | 4 B  | `SerializableMessageType` enum value (little-endian uint32) |
 * | 4      | N B  | Payload produced by `createSerializedContent()` |
 *
 * **Sending** — call `getSerializedMessage()` to obtain the complete framed `MemoryBlock`,
 * then pass it to `InterprocessConnection::sendMessage()`.
 *
 * **Receiving** — pass the raw `MemoryBlock` received from the socket to the static
 * `initFromMemoryBlock()` factory, which reads the type byte and constructs the correct
 * concrete subclass.  The caller owns the returned pointer and must call `freeMessageData()`
 * to properly destroy it via the correct subclass destructor.
 *
 * **Echo suppression** — the optional user-id (`m_userId`) is set by `MemaProcessor` when
 * it forwards an inbound `ControlParametersMessage` back out to all other connected clients.
 * The originating client's connection-id is stored so that the server-side commander wrapper
 * can skip re-sending the update to the client that triggered it.
 *
 * @see InterprocessConnectionImpl  — client-side TCP wrapper used by Mema.Mo and Mema.Re.
 * @see InterprocessConnectionServerImpl — server-side TCP wrapper used by MemaProcessor.
 * @see MemaProcessor::handleMessage() — dispatches received messages on the JUCE message thread.
 */
class SerializableMessage : public juce::Message
{
public:
    enum SerializableMessageType
    {
        None = 0,                    ///< Sentinel / uninitialised type.
        EnvironmentParameters,       ///< Look-and-feel palette sent by Mema to clients on connect.
        AnalyzerParameters,          ///< Audio device sample rate and block size; lets clients initialise their local ProcessorDataAnalyzer.
        ReinitIOCount,               ///< New input/output channel count; clients must rebuild their UI accordingly.
        AudioInputBuffer,            ///< Raw PCM input buffer streamed from Mema to subscribed clients.
        AudioOutputBuffer,           ///< Raw PCM output buffer streamed from Mema to subscribed clients.
        DataTrafficTypeSelection,    ///< Sent by a client to opt in/out of specific message types (bandwidth control).
        ControlParameters,           ///< Full routing-matrix state snapshot; sent by Mema on connect and echoed by Mema.Re on change.
        PluginParameterInfos,        ///< Plugin name and full parameter descriptor list; sent by Mema when a plugin is loaded or changed.
        PluginParameterValue,        ///< Single parameter value update sent from Mema.Re to Mema.
        PluginProcessingState        ///< Plugin enabled and pre/post processing state; sent bidirectionally between Mema and Mema.Re.
    };

public:
    SerializableMessage() = default;
    virtual ~SerializableMessage() = default;

    /** @brief Tags the message with a connection-id used for echo-suppression on the server. @param id The connection ID of the originating TCP client (-1 = no origin). */
    void setId(int id) { m_userId = id; };
    /** @brief Returns the connection-id tag, or -1 if not set. */
    int getId() const { return m_userId; };
    /** @brief Returns true when a non-default connection-id has been assigned. */
    bool hasUserId() const { return -1 != m_userId; };

    /** @brief Returns the concrete message type discriminator. */
    const SerializableMessageType getType() const { return m_type; };

    /**
     * @brief Serialises the message to a `MemoryBlock` ready to send over the socket.
     * @details Prepends the `SerializableMessageType` enum value (4 bytes) followed by the
     *          payload produced by the subclass `createSerializedContent()` implementation.
     * @return A heap-allocated `MemoryBlock` containing the complete framed message.
     */
    juce::MemoryBlock getSerializedMessage() const
    {
        size_t contentSize = 0;
        juce::MemoryBlock blob;
        blob.append(&m_type, sizeof(SerializableMessageType));
        auto sc = createSerializedContent(contentSize);
        blob.append(sc.getData(), contentSize);
        return blob;
    };
    /**
     * @brief Deserialises a raw TCP frame into the correct concrete `SerializableMessage` subclass.
     * @details Reads the first 4 bytes as a `SerializableMessageType` enum and constructs the
     *          matching subclass using its `MemoryBlock`-taking constructor.  The caller takes
     *          ownership of the returned pointer and must call `freeMessageData()` to destroy it.
     * @param blob The raw frame received from `InterprocessConnection::messageReceived()`.
     * @return Heap-allocated concrete message, or `nullptr` if the type is `None` or unknown.
     */
    static SerializableMessage* initFromMemoryBlock(const juce::MemoryBlock& blob)
    {
        auto minSize = sizeof(SerializableMessageType);
        jassert(blob.getSize() >= minSize);
        if (blob.getSize() < minSize)
            return nullptr;

        auto type = static_cast<SerializableMessageType>(blob[0]);
        switch (type)
        {
        case EnvironmentParameters:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<EnvironmentParametersMessage>(blob).release());
        case AnalyzerParameters:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<AnalyzerParametersMessage>(blob).release());
        case ReinitIOCount:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<ReinitIOCountMessage>(blob).release());
        case AudioInputBuffer:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<AudioInputBufferMessage>(blob).release());
        case AudioOutputBuffer:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<AudioOutputBufferMessage>(blob).release());
        case DataTrafficTypeSelection:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<DataTrafficTypeSelectionMessage>(blob).release());
        case ControlParameters:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<ControlParametersMessage>(blob).release());
        case PluginParameterInfos:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<PluginParameterInfosMessage>(blob).release());
        case PluginParameterValue:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<PluginParameterValueMessage>(blob).release());
        case PluginProcessingState:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<PluginProcessingStateMessage>(blob).release());
        case None:
        default:
            return nullptr;
        }
    };
    /**
     * @brief Type-correctly destroys a `SerializableMessage*` returned by `initFromMemoryBlock()`.
     * @details Because the pointer is typed as the base class, a plain `delete` would invoke the
     *          wrong destructor.  This helper casts to the concrete subclass before destruction.
     * @param message The message to destroy.  Safe to call with `nullptr`.
     */
    static void freeMessageData(SerializableMessage* message)
    {
        if (nullptr != message)
        {
            switch (message->getType())
            {
            case AnalyzerParameters:
                {
                    auto apm = std::unique_ptr<AnalyzerParametersMessage>(reinterpret_cast<AnalyzerParametersMessage*>(message));
                }
                break;
            case ReinitIOCount:
                {
                    auto riocm = std::unique_ptr<ReinitIOCountMessage>(reinterpret_cast<ReinitIOCountMessage*>(message));
                }
                break;
            case AudioInputBuffer:
                {
                    auto aibm = std::unique_ptr<AudioInputBufferMessage>(reinterpret_cast<AudioInputBufferMessage*>(message));
                }
                break;
            case AudioOutputBuffer:
                {
                    auto aobm = std::unique_ptr<AudioOutputBufferMessage>(reinterpret_cast<AudioOutputBufferMessage*>(message));
                }
                break;
            case DataTrafficTypeSelection:
                {
                    auto dtsm = std::unique_ptr<DataTrafficTypeSelectionMessage>(reinterpret_cast<DataTrafficTypeSelectionMessage*>(message));
                }
                break;
            case ControlParameters:
                {
                    auto cpm = std::unique_ptr<ControlParametersMessage>(reinterpret_cast<ControlParametersMessage*>(message));
                }
                break;
            case PluginParameterInfos:
                {
                    auto ppim = std::unique_ptr<PluginParameterInfosMessage>(reinterpret_cast<PluginParameterInfosMessage*>(message));
                }
                break; 
            case PluginParameterValue:
                {
                    auto ppvm = std::unique_ptr<PluginParameterValueMessage>(reinterpret_cast<PluginParameterValueMessage*>(message));
                }
                break;
            case PluginProcessingState:
                {
                    auto pesm = std::unique_ptr<PluginProcessingStateMessage>(reinterpret_cast<PluginProcessingStateMessage*>(message));
                }
                break;
            case None:
            default:
                break;
            }
        }
    };

protected:
    //==============================================================================
    /**
     * @brief Subclass hook — produces the type-specific payload bytes (everything after the type discriminator).
     * @param contentSize Set by the implementation to the byte count of the returned block.
     * @return A `MemoryBlock` containing only the payload (no type prefix).
     */
    virtual juce::MemoryBlock createSerializedContent(size_t& contentSize) const = 0;

    //==============================================================================
    /** @brief Reads a big-endian uint32 from @p buffer. @param buffer Pointer to at least 4 bytes of raw data. @return The decoded value. */
    std::uint32_t ReadUint32(const char* buffer)
    {
        return (((static_cast<std::uint8_t>(buffer[0]) << 24) & 0xff000000) +
            ((static_cast<std::uint8_t>(buffer[1]) << 16) & 0x00ff0000) +
            ((static_cast<std::uint8_t>(buffer[2]) << 8) & 0x0000ff00) +
            static_cast<std::uint8_t>(buffer[3]));
    };
    /** @brief Reads a big-endian uint16 from @p buffer. @param buffer Pointer to at least 2 bytes of raw data. @return The decoded value. */
    std::uint16_t ReadUint16(const char* buffer)
    {
        return (((static_cast<std::uint8_t>(buffer[0]) << 8) & 0xff00) +
            static_cast<std::uint8_t>(buffer[1]));
    };

    //==============================================================================
    SerializableMessageType m_type = SerializableMessageType::None; ///< Type discriminator stored in the first 4 bytes of every serialised frame.
    int m_userId = -1; ///< Optional connection-id tag for echo-suppression (-1 = not set).
};

//==============================================================================
/**
 * @class EnvironmentParametersMessage
 * @brief Carries the active look-and-feel palette style from Mema to connected clients.
 *
 * @details Sent by `MemaProcessor` immediately after a new TCP client connects, so that
 * Mema.Mo and Mema.Re can match Mema's current dark/light palette without manual
 * configuration on the client side.  Also re-sent whenever the user changes the palette
 * in Mema's settings menu.
 *
 * **Wire payload (4 bytes):** `PaletteStyle` enum value.
 */
class EnvironmentParametersMessage : public SerializableMessage
{
public:
    EnvironmentParametersMessage() = default;
    EnvironmentParametersMessage(JUCEAppBasics::CustomLookAndFeel::PaletteStyle paletteStyle) { m_type = SerializableMessageType::EnvironmentParameters; m_paletteStyle = paletteStyle; };
    EnvironmentParametersMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::EnvironmentParameters == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::EnvironmentParameters;
        blob.copyTo(&m_paletteStyle, sizeof(SerializableMessageType), sizeof(JUCEAppBasics::CustomLookAndFeel::PaletteStyle));

    };
    ~EnvironmentParametersMessage() = default;

    /** @brief Returns the palette style carried by this message. @return The look-and-feel palette (dark, light, or follow-host). */
    JUCEAppBasics::CustomLookAndFeel::PaletteStyle getPaletteStyle() const { return m_paletteStyle; };

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;
        blob.append(&m_paletteStyle, sizeof(JUCEAppBasics::CustomLookAndFeel::PaletteStyle));
        contentSize = blob.getSize();
        return blob;
    };

private:
    JUCEAppBasics::CustomLookAndFeel::PaletteStyle m_paletteStyle = JUCEAppBasics::CustomLookAndFeel::PS_Dark; ///< The palette style to apply on the receiving client.
};

//==============================================================================
/**
 * @class AnalyzerParametersMessage
 * @brief Carries audio-device parameters (sample rate, block size) from Mema to clients.
 *
 * @details Sent by `MemaProcessor::audioDeviceAboutToStart()` to every connected client
 * so that each client can initialise its local `ProcessorDataAnalyzer` replica with the
 * same parameters Mema uses internally, ensuring that metering and spectrum data are
 * computed at the correct scale.
 *
 * **Wire payload (4 bytes):** uint16 sampleRate + uint16 maximumExpectedSamplesPerBlock.
 */
class AnalyzerParametersMessage : public SerializableMessage
{
public:
    AnalyzerParametersMessage() = default;
    AnalyzerParametersMessage(int sampleRate, int maximumExpectedSamplesPerBlock) { m_type = SerializableMessageType::AnalyzerParameters; m_sampleRate = std::uint16_t(sampleRate); m_maximumExpectedSamplesPerBlock = std::uint16_t(maximumExpectedSamplesPerBlock); };
    AnalyzerParametersMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::AnalyzerParameters == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::AnalyzerParameters;
        blob.copyTo(&m_sampleRate, sizeof(SerializableMessageType), sizeof(std::uint16_t));
        blob.copyTo(&m_maximumExpectedSamplesPerBlock, sizeof(SerializableMessageType) + sizeof(std::uint16_t), sizeof(std::uint16_t));

    };
    ~AnalyzerParametersMessage() = default;

    /** @brief Returns the audio device sample rate in Hz. */
    int getSampleRate() const { return m_sampleRate; };
    /** @brief Returns the maximum number of samples per processing block. */
    int getMaximumExpectedSamplesPerBlock() const { return m_maximumExpectedSamplesPerBlock; };

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;
        blob.append(&m_sampleRate, sizeof(std::uint16_t));
        blob.append(&m_maximumExpectedSamplesPerBlock, sizeof(std::uint16_t));
        contentSize = blob.getSize();
        return blob;
    };

private:
    std::uint16_t m_sampleRate = 0; ///< Sample rate in Hz (stored as uint16 to save wire bytes; max 65535 Hz).
    std::uint16_t m_maximumExpectedSamplesPerBlock = 0; ///< Max block size in samples.
};

//==============================================================================
/**
 * @class ReinitIOCountMessage
 * @brief Instructs clients to tear down and rebuild their UI for a new channel count.
 *
 * @details Sent by `MemaProcessor` whenever the active audio device changes its reported
 * input or output channel count (e.g. the user selects a different audio interface, or
 * enables/disables channels in the device setup panel).  On receipt, Mema.Mo rebuilds
 * its meterbridge columns and Mema.Re rebuilds its faderbank rows/columns.
 *
 * **Wire payload (4 bytes):** uint16 inputCount + uint16 outputCount.
 */
class ReinitIOCountMessage : public SerializableMessage
{
public:
    ReinitIOCountMessage() = default;
    ReinitIOCountMessage(int inputs, int outputs) { m_type = SerializableMessageType::ReinitIOCount; m_inputCount = std::uint16_t(inputs); m_outputCount = std::uint16_t(outputs); };
    ReinitIOCountMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::ReinitIOCount == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::ReinitIOCount;
        blob.copyTo(&m_inputCount, sizeof(SerializableMessageType), sizeof(std::uint16_t));
        blob.copyTo(&m_outputCount, sizeof(SerializableMessageType) + sizeof(std::uint16_t), sizeof(std::uint16_t));

    };
    ~ReinitIOCountMessage() = default;

    /** @brief Returns the new number of active input channels. */
    std::uint16_t getInputCount() const { return m_inputCount; };
    /** @brief Returns the new number of active output channels. */
    std::uint16_t getOutputCount() const { return m_outputCount; };

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;
        blob.append(&m_inputCount, sizeof(std::uint16_t));
        blob.append(&m_outputCount, sizeof(std::uint16_t));
        contentSize = blob.getSize();
        return blob;
    };

private:
    std::uint16_t m_inputCount = 0; ///< New active input channel count.
    std::uint16_t m_outputCount = 0; ///< New active output channel count.
};

//==============================================================================
/**
 * @class AudioBufferMessage
 * @brief Base message carrying a serialised audio buffer and its flow-direction metadata.
 *
 * @details The audio buffer is flattened channel-by-channel into the wire payload as raw
 * IEEE-754 float samples.  Clients use the received buffer to feed their local
 * `ProcessorDataAnalyzer` for level metering and spectrum analysis.
 *
 * **Wire payload:** `FlowDirection` (4 B) + numChannels (uint16) + numSamples (uint16)
 * + numChannels × numSamples × 4 B of float sample data.
 *
 * @note Audio buffers are streamed continuously at the audio device's block rate.
 *       Clients must subscribe via `DataTrafficTypeSelectionMessage` to opt in to receiving
 *       `AudioInputBuffer` or `AudioOutputBuffer` messages; without a subscription Mema will
 *       not send audio data to that client.
 */
class AudioBufferMessage : public SerializableMessage
{
public:
    /** @brief Identifies whether the buffer contains pre-matrix input or post-matrix output samples. */
    enum FlowDirection
    {
        Invalid, ///< Uninitialised direction.
        Input,   ///< Pre-matrix input samples (as seen by the input analyzers).
        Output,  ///< Post-matrix output samples (as seen by the output analyzers).
    };

public:
    AudioBufferMessage() = default;
    AudioBufferMessage(juce::AudioBuffer<float>& buffer) { m_buffer = buffer; };
    ~AudioBufferMessage() = default;

    /** @brief Returns a const reference to the decoded audio buffer. */
    const juce::AudioBuffer<float>& getAudioBuffer() const { return m_buffer; };
    /** @brief Returns the flow direction encoded in the message. */
    const FlowDirection getFlowDirection() const { return m_direction; };

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const {
        auto numChannels = std::uint16_t(m_buffer.getNumChannels());
        auto numSamples = std::uint16_t(m_buffer.getNumSamples());
        juce::MemoryBlock blob;
        blob.append(&m_direction, sizeof(FlowDirection));
        blob.append(&numChannels, sizeof(std::uint16_t));
        blob.append(&numSamples, sizeof(std::uint16_t));
        for (int channelNumber = 0; channelNumber < numChannels; channelNumber++)
            blob.append(m_buffer.getReadPointer(channelNumber), sizeof(float) * numSamples);
        contentSize = blob.getSize();

        return blob;
    };

    FlowDirection               m_direction{ FlowDirection::Invalid }; ///< Input or output flow direction.
    juce::AudioBuffer<float>    m_buffer; ///< Decoded float audio buffer.

};

/**
 * @class AudioInputBufferMessage
 * @brief Carries a pre-matrix input audio buffer streamed continuously from Mema to subscribed clients.
 *
 * @details Instantiated by `MemaProcessor` in `audioDeviceIOCallbackWithContext()` after the input
 * data has been captured but before it passes through the crosspoint matrix.  Clients that have
 * subscribed to `AudioInputBuffer` traffic (via `DataTrafficTypeSelectionMessage`) receive one
 * of these per audio device block.
 */
class AudioInputBufferMessage : public AudioBufferMessage
{
public:
    AudioInputBufferMessage() = default;
    AudioInputBufferMessage(juce::AudioBuffer<float>& buffer) : AudioBufferMessage(buffer) { m_type = SerializableMessageType::AudioInputBuffer; m_direction = FlowDirection::Input; };
    AudioInputBufferMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::AudioInputBuffer == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::AudioInputBuffer;

        auto readPos = int(sizeof(SerializableMessageType));

        blob.copyTo(&m_direction, readPos, sizeof(FlowDirection));
        jassert(FlowDirection::Input == m_direction);

        readPos += sizeof(FlowDirection);
        auto numChannels = std::uint16_t(0);
        blob.copyTo(&numChannels, readPos, sizeof(std::uint16_t));
        readPos += sizeof(std::uint16_t);
        auto numSamples = std::uint16_t(0);
        blob.copyTo(&numSamples, readPos, sizeof(std::uint16_t));
        readPos += sizeof(std::uint16_t);
        auto data = reinterpret_cast<const float*>(blob.begin() + readPos);

        m_buffer = juce::AudioBuffer<float>(numChannels, numSamples);
        auto samplePos = 0;
        for (int i = 0; i < numChannels; i++)
        {
            m_buffer.copyFrom(i, 0, data + samplePos, numSamples);
            samplePos += numSamples;
        }
    };
    ~AudioInputBufferMessage() = default;
};

/**
 * @class AudioOutputBufferMessage
 * @brief Carries a post-matrix output audio buffer streamed continuously from Mema to subscribed clients.
 *
 * @details Instantiated by `MemaProcessor` in `audioDeviceIOCallbackWithContext()` after the signal
 * has passed through all mutes, the crosspoint gain matrix, and the optional plugin.  Mema.Mo's
 * `MemaMoComponent` feeds these buffers directly into its local `ProcessorDataAnalyzer` to drive
 * metering and spectrum visualisation without requiring a second audio device on the monitoring machine.
 */
class AudioOutputBufferMessage : public AudioBufferMessage
{
public:
    AudioOutputBufferMessage() = default;
    AudioOutputBufferMessage(juce::AudioBuffer<float>& buffer) : AudioBufferMessage(buffer) { m_type = SerializableMessageType::AudioOutputBuffer; m_direction = FlowDirection::Output; };
    AudioOutputBufferMessage(const juce::MemoryBlock& blob)
    {
        m_type = SerializableMessageType::AudioOutputBuffer;

        auto readPos = int(sizeof(SerializableMessageType));

        blob.copyTo(&m_direction, readPos, sizeof(FlowDirection));
        jassert(FlowDirection::Output == m_direction);

        readPos += sizeof(FlowDirection);
        auto numChannels = std::uint16_t(0);
        blob.copyTo(&numChannels, readPos, sizeof(std::uint16_t));
        readPos += sizeof(std::uint16_t);
        auto numSamples = std::uint16_t(0);
        blob.copyTo(&numSamples, readPos, sizeof(std::uint16_t));
        readPos += sizeof(std::uint16_t);
        auto data = reinterpret_cast<const float*>(blob.begin() + readPos);

        m_buffer = juce::AudioBuffer<float>(numChannels, numSamples);
        auto samplePos = 0;
        for (int i = 0; i < numChannels; i++)
        {
            m_buffer.copyFrom(i, 0, data + samplePos, numSamples);
            samplePos += numSamples;
        }
    };
    ~AudioOutputBufferMessage() = default;
};

//==============================================================================
/**
 * @class DataTrafficTypeSelectionMessage
 * @brief Sent by a client to opt in to receiving specific message types from Mema.
 *
 * @details Because audio buffers are large and high-frequency, Mema does not broadcast them
 * to all connected clients by default.  A newly connected client sends this message to declare
 * which `SerializableMessageType` values it wants to receive.
 *
 * - **Mema.Mo** subscribes to `AudioOutputBuffer` (and optionally `AudioInputBuffer`) to
 *   drive its local `ProcessorDataAnalyzer`.
 * - **Mema.Re** subscribes to `ControlParameters` and `PluginParameterInfos` only — it
 *   never requests audio buffers.
 *
 * **Wire payload:** uint16 typesCount + typesCount × `SerializableMessageType` (4 B each).
 */
class DataTrafficTypeSelectionMessage : public SerializableMessage
{
public:
    DataTrafficTypeSelectionMessage() = default;
    DataTrafficTypeSelectionMessage(const std::vector<SerializableMessageType>& trafficTypes) { m_type = SerializableMessageType::DataTrafficTypeSelection; m_trafficTypes = trafficTypes; };
    DataTrafficTypeSelectionMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::DataTrafficTypeSelection == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::DataTrafficTypeSelection;

        auto readPos = int(sizeof(SerializableMessageType));

        std::uint16_t typesCount;
        blob.copyTo(&typesCount, readPos, sizeof(std::uint16_t));
        readPos += sizeof(std::uint16_t);
        m_trafficTypes.resize(typesCount);
        for (int i = 0; i < typesCount; i++)
        {
            blob.copyTo(&m_trafficTypes[i], readPos, sizeof(SerializableMessageType));
            readPos += sizeof(SerializableMessageType);
        }

    };
    ~DataTrafficTypeSelectionMessage() = default;

    /** @brief Returns the list of message types this client wants to receive. */
    const std::vector<SerializableMessageType>& getTrafficTypes() const { return m_trafficTypes; };

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;
        auto typesCount = std::uint16_t(m_trafficTypes.size());
        blob.append(&typesCount, sizeof(std::uint16_t));
        for (auto& trafficType : m_trafficTypes)
            blob.append(&trafficType, sizeof(SerializableMessageType));
        contentSize = blob.getSize();
        return blob;
    };

private:
    std::vector<SerializableMessageType>    m_trafficTypes; ///< Ordered list of `SerializableMessageType` values this client has subscribed to.
};

/**
 * @class ControlParametersMessage
 * @brief Full routing-matrix state snapshot exchanged bidirectionally between Mema and Mema.Re.
 *
 * @details This message is the primary vehicle for remote control in the Mema tool suite:
 *
 * **Mema → Mema.Re (on connect):**
 * `MemaProcessor` sends the current complete state immediately after a Mema.Re client connects
 * and subscribes.  This initialises the Mema.Re UI to match the live server state without
 * requiring the user to manually synchronise.
 *
 * **Mema.Re → Mema (on user interaction):**
 * Whenever the user changes a mute, crosspoint enable, or crosspoint gain in Mema.Re, the
 * `MemaReComponent` serialises the full updated state into a new `ControlParametersMessage`
 * and sends it via `onMessageReadyToSend`.  `MemaProcessor::handleMessage()` unpacks it and
 * applies each changed value, then echoes the updated state to all *other* connected clients
 * (echo-suppression via `m_userId`).
 *
 * **Wire payload:**
 * - uint16 inputMuteCount + (uint16 channel + bool muted) × N
 * - uint16 outputMuteCount + (uint16 channel + bool muted) × M
 * - uint16 crosspointStateCount + (uint16 in + uint16 out + bool enabled) × (N×M)
 * - uint16 crosspointValueCount + (uint16 in + uint16 out + float gain) × (N×M)
 *
 * @note Channel indices are 1-based and match the indices used by `MemaInputCommander` /
 *       `MemaOutputCommander` / `MemaCrosspointCommander`.
 * @see MemaProcessor::handleMessage()
 * @see MemaReComponent::handleMessage()
 */
class ControlParametersMessage : public SerializableMessage
{
public:
    ControlParametersMessage() = default;
    ControlParametersMessage(const std::map<std::uint16_t, bool>& inputMuteStates, const std::map<std::uint16_t, bool>& outputMuteStates, 
        const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates, const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues)
    {
#ifdef DEBUG
        // sanity check symmetry of crosspoint states
        auto crosspointStateOutCount = size_t(0);
        if (0 != crosspointStates.size())
        {
            crosspointStateOutCount = crosspointStates.begin()->second.size();
            for (auto const& cpStatKV : crosspointStates)
            {
                jassert(crosspointStateOutCount == cpStatKV.second.size());
            }
        }

        // sanity check symmetry of crosspoint values
        auto crosspointValOutCount = size_t(0);
        if (0 != crosspointValues.size())
        {
            crosspointValOutCount = crosspointValues.begin()->second.size();
            for (auto const& cpValKV : crosspointValues)
            {
                jassert(crosspointValOutCount == cpValKV.second.size());
            }
        }
#endif

        m_type = SerializableMessageType::ControlParameters;
        m_inputMuteStates = inputMuteStates;
        m_outputMuteStates = outputMuteStates;
        m_crosspointStates = crosspointStates;
        m_crosspointValues = crosspointValues;
    };
    ControlParametersMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::ControlParameters == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::ControlParameters;

        auto readPos = int(sizeof(SerializableMessageType));

        std::uint16_t inputMuteStatesCount;
        blob.copyTo(&inputMuteStatesCount, readPos, sizeof(std::uint16_t));
        readPos += sizeof(inputMuteStatesCount);
        for (int i = 0; i < inputMuteStatesCount; i++)
        {
            std::pair<std::uint16_t, bool> inputMuteState;
            blob.copyTo(&inputMuteState.first, readPos, sizeof(inputMuteState.first));
            readPos += sizeof(inputMuteState.first);
            blob.copyTo(&inputMuteState.second, readPos, sizeof(inputMuteState.second));
            readPos += sizeof(inputMuteState.second);

            m_inputMuteStates[inputMuteState.first] = inputMuteState.second;
        }

        std::uint16_t outputMuteStatesCount;
        blob.copyTo(&outputMuteStatesCount, readPos, sizeof(std::uint16_t));
        readPos += sizeof(outputMuteStatesCount);
        for (int i = 0; i < outputMuteStatesCount; i++)
        {
            std::pair<std::uint16_t, bool> outputMuteState;
            blob.copyTo(&outputMuteState.first, readPos, sizeof(outputMuteState.first));
            readPos += sizeof(outputMuteState.first);
            blob.copyTo(&outputMuteState.second, readPos, sizeof(outputMuteState.second));
            readPos += sizeof(outputMuteState.second);

            m_outputMuteStates[outputMuteState.first] = outputMuteState.second;
        }

        std::uint16_t crosspointStatesCount;
        blob.copyTo(&crosspointStatesCount, readPos, sizeof(std::uint16_t));
        readPos += sizeof(crosspointStatesCount);
        for (int i = 0; i < crosspointStatesCount; i++)
        {
            std::uint16_t in, out;
            bool state;
            blob.copyTo(&in, readPos, sizeof(in));
            readPos += sizeof(in);
            blob.copyTo(&out, readPos, sizeof(out));
            readPos += sizeof(out);
            blob.copyTo(&state, readPos, sizeof(state));
            readPos += sizeof(state);

            m_crosspointStates[in][out] = state;
        }

        std::uint16_t crosspointValuesCount;
        blob.copyTo(&crosspointValuesCount, readPos, sizeof(std::uint16_t));
        readPos += sizeof(crosspointValuesCount);
        for (int i = 0; i < crosspointValuesCount; i++)
        {
            std::uint16_t in, out;
            float value;
            blob.copyTo(&in, readPos, sizeof(in));
            readPos += sizeof(in);
            blob.copyTo(&out, readPos, sizeof(out));
            readPos += sizeof(out);
            blob.copyTo(&value, readPos, sizeof(value));
            readPos += sizeof(value);

            m_crosspointValues[in][out] = value;
        }
    };
    ~ControlParametersMessage() = default;

    /** @brief Returns the per-input mute states (channel index → muted). */
    const std::map<std::uint16_t, bool>& getInputMuteStates() const { return m_inputMuteStates; };
    /** @brief Returns the per-output mute states (channel index → muted). */
    const std::map<std::uint16_t, bool>& getOutputMuteStates() const { return m_outputMuteStates; };
    /** @brief Returns the crosspoint enable matrix (input index → output index → enabled). */
    const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& getCrosspointStates() const { return m_crosspointStates; };
    /** @brief Returns the crosspoint gain matrix (input index → output index → linear gain [0, 1]). */
    const std::map<std::uint16_t, std::map<std::uint16_t, float>>& getCrosspointValues() const { return m_crosspointValues; };

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;

        auto inputMuteStatesCount = std::uint16_t(m_inputMuteStates.size());
        blob.append(&inputMuteStatesCount, sizeof(inputMuteStatesCount));
        for (auto& inputMuteStateKV : m_inputMuteStates)
        {
            blob.append(&inputMuteStateKV.first, sizeof(inputMuteStateKV.first));
            blob.append(&inputMuteStateKV.second, sizeof(inputMuteStateKV.second));
        }

        auto outputMuteStatesCount = std::uint16_t(m_outputMuteStates.size());
        blob.append(&outputMuteStatesCount, sizeof(outputMuteStatesCount));
        for (auto& outputMuteStateKV : m_outputMuteStates)
        {
            blob.append(&outputMuteStateKV.first, sizeof(outputMuteStateKV.first));
            blob.append(&outputMuteStateKV.second, sizeof(outputMuteStateKV.second));
        }

        auto crosspointStatesCount = std::uint16_t(0);
        if (0 < m_crosspointStates.size())
            crosspointStatesCount = std::uint16_t(m_crosspointStates.size() * m_crosspointStates.begin()->second.size());
        blob.append(&crosspointStatesCount, sizeof(crosspointStatesCount));
        auto crosspointStatesCountRef = std::uint16_t(0);
        for (auto& crosspointStatesFirstDKV : m_crosspointStates)
        {
            for (auto& crosspointStatesSecDKV : crosspointStatesFirstDKV.second)
            {
                auto& in = crosspointStatesFirstDKV.first;
                blob.append(&in, sizeof(in));
                auto& out = crosspointStatesSecDKV.first;
                blob.append(&out, sizeof(out));
                auto& state = crosspointStatesSecDKV.second;
                blob.append(&state, sizeof(state));
                crosspointStatesCountRef++;
            }
        }
        jassert(crosspointStatesCount == crosspointStatesCountRef);

        auto crosspointValuesCount = std::uint16_t(0);
        if (0 < m_crosspointValues.size())
            crosspointValuesCount = std::uint16_t(m_crosspointValues.size() * m_crosspointValues.begin()->second.size());
        blob.append(&crosspointValuesCount, sizeof(crosspointValuesCount));
        auto crosspointValuesCountRef = std::uint16_t(0);
        for (auto& crosspointValuesFirstDKV : m_crosspointValues)
        {
            for (auto& crosspointValuesSecDKV : crosspointValuesFirstDKV.second)
            {
                auto& in = crosspointValuesFirstDKV.first;
                blob.append(&in, sizeof(in));
                auto& out = crosspointValuesSecDKV.first;
                blob.append(&out, sizeof(out));
                auto& value = crosspointValuesSecDKV.second;
                blob.append(&value, sizeof(value));
                crosspointValuesCountRef++;
            }
        }
        jassert(crosspointValuesCount == crosspointValuesCountRef);

        contentSize = blob.getSize();
        return blob;
    };

private:
    std::map<std::uint16_t, bool>                           m_inputMuteStates; ///< Per-input mute state (1-based channel index → muted flag).
    std::map<std::uint16_t, bool>                           m_outputMuteStates; ///< Per-output mute state.
    std::map<std::uint16_t, std::map<std::uint16_t, bool>>  m_crosspointStates; ///< Crosspoint enable matrix [in][out] → bool.
    std::map<std::uint16_t, std::map<std::uint16_t, float>> m_crosspointValues; ///< Crosspoint linear gain matrix [in][out] → float.
};

/**
 * @class PluginParameterInfosMessage
 * @brief Carries the plugin name and complete parameter descriptor list from Mema to Mema.Re clients.
 *
 * @details Sent by `MemaProcessor` to all connected Mema.Re clients whenever a plugin is
 * loaded, unloaded, or its remote-controllability settings change.  On receipt, `MemaReComponent`
 * passes the descriptors to `PluginControlComponent`, which dynamically builds a control widget
 * (slider, combo box, or toggle button) for each `PluginParameterInfo` whose
 * `isRemoteControllable` flag is set.
 *
 * **Wire payload per parameter:** index (int32) + id (uint16 len + UTF-8) + name (uint16 len + UTF-8)
 * + defaultValue (float) + currentValue (float) + label (uint16 len + UTF-8)
 * + isAutomatable (bool) + isRemoteControllable (bool) + category (int32)
 * + minValue (float) + maxValue (float) + stepSize (float) + isDiscrete (bool)
 * + type (`ParameterControlType`) + stepCount (int32) + stepCount × stepName strings.
 *
 * @see PluginParameterInfo — the per-parameter descriptor struct.
 * @see MemaReComponent::handleMessage() — unpacks this message and forwards to PluginControlComponent.
 */
class PluginParameterInfosMessage : public SerializableMessage
{
public:
    PluginParameterInfosMessage() = default;
    PluginParameterInfosMessage(const std::string& pluginName, bool pluginEnabled, bool pluginPost, const std::vector<PluginParameterInfo>& parameterInfos)
    {
        m_type = SerializableMessageType::PluginParameterInfos;
        m_parameterInfos = parameterInfos;
        m_pluginName = pluginName;
        m_pluginEnabled = pluginEnabled;
        m_pluginPost = pluginPost;
    }

    PluginParameterInfosMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::PluginParameterInfos == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::PluginParameterInfos;

        auto readPos = int(sizeof(SerializableMessageType));

        // Read plugin string length and string
        std::uint16_t pluginNameLength;
        blob.copyTo(&pluginNameLength, readPos, sizeof(std::uint16_t));
        readPos += sizeof(std::uint16_t);
        m_pluginName = juce::String(juce::CharPointer_UTF8(static_cast<const char*>(blob.begin()) + readPos), pluginNameLength);
        readPos += pluginNameLength;

        // Read enabled and post state
        blob.copyTo(&m_pluginEnabled, readPos, sizeof(bool));
        readPos += sizeof(bool);
        blob.copyTo(&m_pluginPost, readPos, sizeof(bool));
        readPos += sizeof(bool);

        std::uint16_t paramCount;
        blob.copyTo(&paramCount, readPos, sizeof(std::uint16_t));
        readPos += sizeof(std::uint16_t);

        m_parameterInfos.reserve(paramCount);

        for (int i = 0; i < paramCount; i++)
        {
            PluginParameterInfo info;

            // Read index
            std::int32_t index;
            blob.copyTo(&index, readPos, sizeof(std::int32_t));
            info.index = index;
            readPos += sizeof(std::int32_t);

            // Read id string length and string
            std::uint16_t idLength;
            blob.copyTo(&idLength, readPos, sizeof(std::uint16_t));
            readPos += sizeof(std::uint16_t);
            info.id = juce::String(juce::CharPointer_UTF8(static_cast<const char*>(blob.begin()) + readPos), idLength);
            readPos += idLength;

            // Read name string length and string
            std::uint16_t nameLength;
            blob.copyTo(&nameLength, readPos, sizeof(std::uint16_t));
            readPos += sizeof(std::uint16_t);
            info.name = juce::String(juce::CharPointer_UTF8(static_cast<const char*>(blob.begin()) + readPos), nameLength);
            readPos += nameLength;

            // Read float values
            blob.copyTo(&info.defaultValue, readPos, sizeof(float));
            readPos += sizeof(float);
            blob.copyTo(&info.currentValue, readPos, sizeof(float));
            readPos += sizeof(float);

            // Read label string length and string
            std::uint16_t labelLength;
            blob.copyTo(&labelLength, readPos, sizeof(std::uint16_t));
            readPos += sizeof(std::uint16_t);
            info.label = juce::String(juce::CharPointer_UTF8(static_cast<const char*>(blob.begin()) + readPos), labelLength);
            readPos += labelLength;

            // Read bool values
            blob.copyTo(&info.isAutomatable, readPos, sizeof(bool));
            readPos += sizeof(bool);
            blob.copyTo(&info.isRemoteControllable, readPos, sizeof(bool));
            readPos += sizeof(bool);

            // Read category as int
            std::int32_t categoryInt;
            blob.copyTo(&categoryInt, readPos, sizeof(std::int32_t));
            info.category = static_cast<juce::AudioProcessorParameter::Category>(categoryInt);
            readPos += sizeof(std::int32_t);

            // Read range values
            blob.copyTo(&info.minValue, readPos, sizeof(float));
            readPos += sizeof(float);
            blob.copyTo(&info.maxValue, readPos, sizeof(float));
            readPos += sizeof(float);
            blob.copyTo(&info.stepSize, readPos, sizeof(float));
            readPos += sizeof(float);
            blob.copyTo(&info.isDiscrete, readPos, sizeof(bool));
            readPos += sizeof(bool);

            // Read type
            blob.copyTo(&info.type, readPos, sizeof(ParameterControlType));
            readPos += sizeof(ParameterControlType);

            // Read stepCount
            std::int32_t stepCount;
            blob.copyTo(&stepCount, readPos, sizeof(std::int32_t));
            info.stepCount = stepCount;
            readPos += sizeof(std::int32_t);

            // Read stepNames (count already known from stepCount)
            for (int s = 0; s < stepCount; s++)
            {
                std::uint16_t stepNameLength;
                blob.copyTo(&stepNameLength, readPos, sizeof(std::uint16_t));
                readPos += sizeof(std::uint16_t);
                auto stepName = juce::String(juce::CharPointer_UTF8(
                    static_cast<const char*>(blob.begin()) + readPos), stepNameLength);
                readPos += stepNameLength;
                info.stepNames.push_back(stepName.toStdString());
            }

            m_parameterInfos.push_back(info);
        }
    }

    ~PluginParameterInfosMessage() = default;

    /** @brief Returns the display name of the currently loaded plugin. @return An empty string if no plugin is loaded. */
    const juce::String& getPluginName() const { return m_pluginName; }
    /** @brief Returns the ordered list of parameter descriptors for the loaded plugin. */
    const std::vector<PluginParameterInfo>& getParameterInfos() const { return m_parameterInfos; }
    /** @brief Returns whether plugin processing is currently enabled. */
    bool isPluginEnabled() const { return m_pluginEnabled; }
    /** @brief Returns whether the plugin is inserted post-matrix (true) or pre-matrix (false). */
    bool isPluginPost() const { return m_pluginPost; }

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;

        // Write name string (length + UTF8 bytes)
        auto pluginNameUtf8 = m_pluginName.toUTF8();
        std::uint16_t pluginNameLength = std::uint16_t(strlen(pluginNameUtf8));
        blob.append(&pluginNameLength, sizeof(std::uint16_t));
        blob.append(pluginNameUtf8, pluginNameLength);

        // Write enabled and post state
        blob.append(&m_pluginEnabled, sizeof(bool));
        blob.append(&m_pluginPost, sizeof(bool));

        auto paramCount = std::uint16_t(m_parameterInfos.size());
        blob.append(&paramCount, sizeof(std::uint16_t));

        for (const auto& info : m_parameterInfos)
        {
            // Write index
            std::int32_t index = info.index;
            blob.append(&index, sizeof(std::int32_t));

            // Write id string (length + UTF8 bytes)
            auto idUtf8 = info.id.toUTF8();
            std::uint16_t idLength = std::uint16_t(strlen(idUtf8));
            blob.append(&idLength, sizeof(std::uint16_t));
            blob.append(idUtf8, idLength);

            // Write name string (length + UTF8 bytes)
            auto nameUtf8 = info.name.toUTF8();
            std::uint16_t nameLength = std::uint16_t(strlen(nameUtf8));
            blob.append(&nameLength, sizeof(std::uint16_t));
            blob.append(nameUtf8, nameLength);

            // Write float values
            blob.append(&info.defaultValue, sizeof(float));
            blob.append(&info.currentValue, sizeof(float));

            // Write label string (length + UTF8 bytes)
            auto labelUtf8 = info.label.toUTF8();
            std::uint16_t labelLength = std::uint16_t(strlen(labelUtf8));
            blob.append(&labelLength, sizeof(std::uint16_t));
            blob.append(labelUtf8, labelLength);

            // Write bool values
            blob.append(&info.isAutomatable, sizeof(bool));
            blob.append(&info.isRemoteControllable, sizeof(bool));

            // Write category as int
            std::int32_t categoryInt = static_cast<std::int32_t>(info.category);
            blob.append(&categoryInt, sizeof(std::int32_t));

            // Write range values
            blob.append(&info.minValue, sizeof(float));
            blob.append(&info.maxValue, sizeof(float));
            blob.append(&info.stepSize, sizeof(float));
            blob.append(&info.isDiscrete, sizeof(bool));

            // Write type
            blob.append(&info.type, sizeof(ParameterControlType));

            // Write stepCount
            std::int32_t stepCount = info.stepCount;
            blob.append(&stepCount, sizeof(std::int32_t));

            // Write stepNames
            for (const auto& stepName : info.stepNames)
            {
                juce::String juceStepName(stepName);
                auto stepNameUtf8 = juceStepName.toUTF8();
                std::uint16_t stepNameLength = std::uint16_t(strlen(stepNameUtf8));
                blob.append(&stepNameLength, sizeof(std::uint16_t));
                blob.append(stepNameUtf8, stepNameLength);
            }
        }

        contentSize = blob.getSize();
        return blob;
    }

private:
    std::vector<PluginParameterInfo>    m_parameterInfos; ///< Ordered parameter descriptors matching the plugin's parameter list.
    juce::String                        m_pluginName; ///< Display name of the loaded plugin (empty if no plugin is loaded).
    bool                                m_pluginEnabled = false; ///< Whether plugin processing is currently enabled.
    bool                                m_pluginPost = false; ///< Whether the plugin is inserted post-matrix (true) or pre-matrix (false).
};

/**
 * @class PluginParameterValueMessage
 * @brief Carries a single normalised plugin parameter value from Mema.Re to Mema.
 *
 * @details Sent by `PluginControlComponent` (Mema.Re side) whenever the user adjusts a
 * plugin parameter widget.  On receipt, `MemaProcessor::handleMessage()` calls
 * `setPluginParameterValue()` which forwards the value to the hosted `AudioPluginInstance`.
 * The parameter is identified by both its index and its stable string ID to guard against
 * index drift if the plugin reports parameters in a different order across sessions.
 *
 * **Wire payload:** parameterIndex (uint16) + id (uint16 len + UTF-8) + currentValue (float).
 *
 * @note Values are normalised to [0, 1] as required by JUCE's `AudioProcessorParameter::setValue()`.
 * @see MemaProcessor::setPluginParameterValue()
 */
class PluginParameterValueMessage : public SerializableMessage
{
public:
    PluginParameterValueMessage() = default;
    PluginParameterValueMessage(std::uint16_t parameterIndex, const juce::String& parameterId, float value)
    {
        m_type = SerializableMessageType::PluginParameterValue;
        m_parameterIndex = parameterIndex;
        m_parameterId = parameterId;
        m_currentValue = value;
    }

    PluginParameterValueMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::PluginParameterValue == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::PluginParameterValue;

        auto readPos = int(sizeof(SerializableMessageType));

        // Read index
        blob.copyTo(&m_parameterIndex, readPos, sizeof(std::uint16_t));
        readPos += sizeof(std::uint16_t);

        // Read id string length and string
        std::uint16_t idLength;
        blob.copyTo(&idLength, readPos, sizeof(std::uint16_t));
        readPos += sizeof(std::uint16_t);
        m_parameterId = juce::String(juce::CharPointer_UTF8(static_cast<const char*>(blob.begin()) + readPos), idLength);
        readPos += idLength;

        // Read current value
        blob.copyTo(&m_currentValue, readPos, sizeof(float));
        readPos += sizeof(float);
    }

    ~PluginParameterValueMessage() = default;

    /** @brief Returns the zero-based parameter index within the plugin's parameter list. */
    std::uint16_t getParameterIndex() const { return m_parameterIndex; }
    /** @brief Returns the stable string identifier of the parameter (used for cross-session safety). */
    const juce::String& getParameterId() const { return m_parameterId; }
    /** @brief Returns the normalised parameter value in [0, 1]. */
    float getCurrentValue() const { return m_currentValue; }

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;

        // Write index
        blob.append(&m_parameterIndex, sizeof(std::uint16_t));

        // Write id string (length + UTF8 bytes)
        auto idUtf8 = m_parameterId.toUTF8();
        std::uint16_t idLength = std::uint16_t(strlen(idUtf8));
        blob.append(&idLength, sizeof(std::uint16_t));
        blob.append(idUtf8, idLength);

        // Write current value
        blob.append(&m_currentValue, sizeof(float));

        contentSize = blob.getSize();
        return blob;
    }

private:
    std::uint16_t m_parameterIndex = 0; ///< Zero-based index into the plugin's parameter list.
    juce::String m_parameterId; ///< Stable string ID for cross-session safety.
    float m_currentValue = 0.0f; ///< Normalised value in [0, 1].
};


/**
 * @class PluginProcessingStateMessage
 * @brief Carries the plugin enabled and pre/post insertion state between Mema and Mema.Re.
 *
 * @details Sent by `MemaProcessor` to all connected Mema.Re clients whenever the plugin enabled
 * or pre/post state changes.  Also sent from `PluginControlComponent` (Mema.Re side) back to
 * Mema when the user clicks the enabled or pre/post buttons.
 *
 * **Wire payload:** enabled (bool) + post (bool).
 */
class PluginProcessingStateMessage : public SerializableMessage
{
public:
    PluginProcessingStateMessage() = default;
    PluginProcessingStateMessage(bool enabled, bool post)
    {
        m_type = SerializableMessageType::PluginProcessingState;
        m_enabled = enabled;
        m_post = post;
    }

    PluginProcessingStateMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::PluginProcessingState == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::PluginProcessingState;

        auto readPos = int(sizeof(SerializableMessageType));

        blob.copyTo(&m_enabled, readPos, sizeof(bool));
        readPos += sizeof(bool);
        blob.copyTo(&m_post, readPos, sizeof(bool));
        readPos += sizeof(bool);
    }

    ~PluginProcessingStateMessage() = default;

    /** @brief Returns whether plugin processing is enabled. */
    bool isEnabled() const { return m_enabled; }
    /** @brief Returns whether the plugin is inserted post-matrix (true) or pre-matrix (false). */
    bool isPost() const { return m_post; }

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;
        blob.append(&m_enabled, sizeof(bool));
        blob.append(&m_post, sizeof(bool));
        contentSize = blob.getSize();
        return blob;
    }

private:
    bool m_enabled = false; ///< Whether plugin processing is enabled.
    bool m_post = false; ///< Whether the plugin is inserted post-matrix.
};


#ifdef NIX // DEBUG
#define RUN_MESSAGE_TESTS
#endif
#ifdef RUN_MESSAGE_TESTS
static void runTests()
{
    auto inputs = 11;
    auto outputs = 12;
    auto buffer = juce::AudioBuffer<float>();
    auto refSample = 11.11f;
    auto sr = 48000;
    auto mespb = 256;

    // test AnalyzerParametersMessage
    auto apm = std::make_unique<AnalyzerParametersMessage>(sr, mespb);
    auto apmb = apm->getSerializedMessage();
    auto apmcpy = AnalyzerParametersMessage(apmb);
    auto test5 = apmcpy.getSampleRate();
    auto test6 = apmcpy.getMaximumExpectedSamplesPerBlock();
    jassert(test5 == sr);
    jassert(test6 == mespb);

    // test ReinitIOCountMessage
    auto rcm = std::make_unique<ReinitIOCountMessage>(inputs, outputs);
    auto rcmb = rcm->getSerializedMessage();
    auto rcmcpy = ReinitIOCountMessage(rcmb);
    auto test7 = rcmcpy.getInputCount();
    auto test8 = rcmcpy.getOutputCount();
    jassert(test7 == inputs);
    jassert(test8 == outputs);

    // test AudioInputBufferMessage
    auto channelCount = 2;
    auto sampleCount = 6;
    buffer.setSize(channelCount, sampleCount, false, true, false);
    for (int i = 0; i < channelCount; i++)
    {
        for (int j = 0; j < sampleCount; j++)
        {
            buffer.setSample(i, j, ++refSample);
        }
    }
    auto rrefSample1 = refSample;
    auto aibm1 = std::make_unique<AudioInputBufferMessage>(buffer);
    for (int i = channelCount - 1; i >= 0; i--)
    {
        for (int j = sampleCount - 1; j >= 0; j--)
        {
            auto test1 = aibm1->getAudioBuffer().getSample(i, j);
            jassert(int(test1) == int(refSample));
            refSample--;
        }
    }
    auto aibmb1 = aibm1->getSerializedMessage();
    auto aibmcpy1 = AudioInputBufferMessage(aibmb1);
    for (int i = channelCount - 1; i >= 0; i--)
    {
        for (int j = sampleCount - 1; j >= 0; j--)
        {
            auto test1 = aibmcpy1.getAudioBuffer().getSample(i, j);
            jassert(int(test1) == int(rrefSample1));
            rrefSample1--;
        }
    }

    // test AudioOutputBufferMessage
    buffer.setSize(channelCount, sampleCount, false, true, false);
    for (int i = 0; i < channelCount; i++)
    {
        for (int j = 0; j < sampleCount; j++)
        {
            buffer.setSample(i, j, ++refSample);
        }
    }
    auto rrefSample2 = refSample;
    auto aibm2 = std::make_unique<AudioOutputBufferMessage>(buffer);
    for (int i = channelCount - 1; i >= 0; i--)
    {
        for (int j = sampleCount - 1; j >= 0; j--)
        {
            auto test2 = aibm2->getAudioBuffer().getSample(i, j);
            jassert(int(test2) == int(rrefSample2));
            rrefSample2--;
        }
    }
    auto aibmb2 = aibm2->getSerializedMessage();
    auto aibmcpy2 = AudioOutputBufferMessage(aibmb2);
    for (int i = channelCount - 1; i >= 0; i--)
    {
        for (int j = sampleCount - 1; j >= 0; j--)
        {
            auto test2 = aibmcpy2.getAudioBuffer().getSample(i, j);
            jassert(int(test2) == int(refSample));
            refSample--;
        }
    }

    // test EnvironmentParametersMessage
    auto paletteStyle = JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Light;
    auto epm = std::make_unique<EnvironmentParametersMessage>(paletteStyle);
    auto epmb = epm->getSerializedMessage();
    auto epmcpy = EnvironmentParametersMessage(epmb);
    auto test9 = epmcpy.getPaletteStyle();
    jassert(test9 == paletteStyle);

    // test DataTrafficTypeSelectionMessage
    auto trafficTypes = std::vector<SerializableMessage::SerializableMessageType>({ SerializableMessage::ControlParameters, SerializableMessage::AnalyzerParameters });
    auto dttm = std::make_unique<DataTrafficTypeSelectionMessage>(trafficTypes);
    auto dttmb = dttm->getSerializedMessage();
    auto dttmcpy = DataTrafficTypeSelectionMessage(dttmb);
    auto test10 = dttmcpy.getTrafficTypes();
    jassert(test10 == trafficTypes);

    // test ControlParametersMessage
    auto inputMuteStates = std::map<std::uint16_t, bool>{ { std::uint16_t(1), true}, { std::uint16_t(2), false}, { std::uint16_t(3), true} };
    auto outputMuteStates = std::map<std::uint16_t, bool>{ { std::uint16_t(4), false}, { std::uint16_t(5), true}, { std::uint16_t(6), false} };
    auto crosspointStates = std::map<std::uint16_t, std::map<std::uint16_t, bool>>();
    auto crosspointValues = std::map<std::uint16_t, std::map<std::uint16_t, float>>();
    crosspointStates[1][1] = false;
    crosspointStates[1][2] = true;
    crosspointStates[2][1] = true;
    crosspointStates[2][2] = true;
    crosspointValues[1][1] = 0.0f;
    crosspointValues[1][2] = 1.0f;
    crosspointValues[2][1] = 0.5f;
    crosspointValues[2][2] = 0.7f;
    auto cpm = std::make_unique<ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
    auto cpmb = cpm->getSerializedMessage();
    auto cpmcpy = ControlParametersMessage(cpmb);
    auto test11 = cpmcpy.getInputMuteStates();
    auto test12 = cpmcpy.getOutputMuteStates();
    auto test13 = cpmcpy.getCrosspointStates();
    auto test14 = cpmcpy.getCrosspointValues();
    jassert(test11 == inputMuteStates);
    jassert(test12 == outputMuteStates);
    jassert(test13 == crosspointStates);
    jassert(test14 == crosspointValues);
}
#endif


};

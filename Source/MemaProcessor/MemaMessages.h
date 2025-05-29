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

namespace Mema
{


//==============================================================================
/*
 *
 */
class EnvironmentParametersMessage;
class AnalyzerParametersMessage;
class ReinitIOCountMessage;
class AudioInputBufferMessage;
class AudioOutputBufferMessage;

class SerializableMessage : public juce::Message
{
public:
    enum SerializableMessageType
    {
        None = 0,
        EnvironmentParameters,
        AnalyzerParameters,
        ReinitIOCount,
        AudioInputBuffer,
        AudioOutputBuffer
    };

public:
    SerializableMessage() = default;
    virtual ~SerializableMessage() = default;

    const SerializableMessageType getType() const { return m_type; };

    juce::MemoryBlock getSerializedMessage() const
    {
        size_t contentSize = 0;
        juce::MemoryBlock blob;
        blob.append(&m_type, sizeof(SerializableMessageType));
        auto sc = createSerializedContent(contentSize);
        blob.append(sc.getData(), contentSize);
        return blob;
    };
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
        case None:
        default:
            return nullptr;
        }
    };
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
            case None:
            default:
                break;
            }
        }
    };

protected:
    //==============================================================================
    virtual juce::MemoryBlock createSerializedContent(size_t& contentSize) const = 0;

    //==============================================================================
    std::uint32_t ReadUint32(const char* buffer)
    {
        return (((static_cast<std::uint8_t>(buffer[0]) << 24) & 0xff000000) +
            ((static_cast<std::uint8_t>(buffer[1]) << 16) & 0x00ff0000) +
            ((static_cast<std::uint8_t>(buffer[2]) << 8) & 0x0000ff00) +
            static_cast<std::uint8_t>(buffer[3]));
    };
    std::uint16_t ReadUint16(const char* buffer)
    {
        return (((static_cast<std::uint8_t>(buffer[0]) << 8) & 0xff00) +
            static_cast<std::uint8_t>(buffer[1]));
    };

    //==============================================================================
    SerializableMessageType m_type = SerializableMessageType::None;
};

//==============================================================================
/*
 *
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
    JUCEAppBasics::CustomLookAndFeel::PaletteStyle m_paletteStyle = JUCEAppBasics::CustomLookAndFeel::PS_Dark;
};

//==============================================================================
/*
 *
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

    int getSampleRate() const { return m_sampleRate; };
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
    std::uint16_t m_sampleRate = 0;
    std::uint16_t m_maximumExpectedSamplesPerBlock = 0;
};

//==============================================================================
/*
 *
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
        blob.copyTo(&m_inputCount, sizeof(SerializableMessageType), 2);
        blob.copyTo(&m_outputCount, sizeof(SerializableMessageType) + sizeof(std::uint16_t), 2);

    };
    ~ReinitIOCountMessage() = default;

    int getInputCount() const { return m_inputCount; };
    int getOutputCount() const { return m_outputCount; };

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
    std::uint16_t m_inputCount = 0;
    std::uint16_t m_outputCount = 0;
};

//==============================================================================
/*
 * 
 */
class AudioBufferMessage : public SerializableMessage
{
public:
    enum FlowDirection
    {
        Invalid,
        Input,
        Output,
    };

public:
    AudioBufferMessage() = default;
    AudioBufferMessage(juce::AudioBuffer<float>& buffer) { m_buffer = buffer; };
    ~AudioBufferMessage() = default;

    const juce::AudioBuffer<float>& getAudioBuffer() const { return m_buffer; };
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

    FlowDirection               m_direction{ FlowDirection::Invalid };
    juce::AudioBuffer<float>    m_buffer;

};

//==============================================================================
/*
 *
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

//==============================================================================
/*
 *
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


};

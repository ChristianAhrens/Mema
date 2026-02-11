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

//==============================================================================
/*
 * Baseclass for all messages that can be used in Memas inter-instance message concept
 */
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
        AudioOutputBuffer,
        DataTrafficTypeSelection,
        ControlParameters,
        PluginParameterInfos,
        PluginParameterValue
    };

public:
    SerializableMessage() = default;
    virtual ~SerializableMessage() = default;

    void setId(int id) { m_userId = id; };
    int getId() const { return m_userId; };
    bool hasUserId() const { return -1 != m_userId; };

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
        case DataTrafficTypeSelection:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<DataTrafficTypeSelectionMessage>(blob).release());
        case ControlParameters:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<ControlParametersMessage>(blob).release());
        case PluginParameterInfos:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<PluginParameterInfosMessage>(blob).release());
        case PluginParameterValue:
            return reinterpret_cast<SerializableMessage*>(std::make_unique<PluginParameterValueMessage>(blob).release());
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
    int m_userId = -1;
};

//==============================================================================
/*
 * A message type encapsulating Mema environment parameters
 * #1 Light/Dark lookandfeel
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
 * A message type encapsulating Analyser parameters
 * #1 Samplerate
 * #2 Max samples per block
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
 * A message type encapsulating IOcount infos to trigger a client reinit
 * #1 new input count
 * #2 new output count
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

    std::uint16_t getInputCount() const { return m_inputCount; };
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
    std::uint16_t m_inputCount = 0;
    std::uint16_t m_outputCount = 0;
};

//==============================================================================
/*
 * A message type encapsulating audio buffer data
 * #1 buffer meta info: in or out
 * #2 actual buffer data
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
 * A message type specializing the AudioBufferMessage to a specific input buffer message
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
 * A message type specializing the AudioBufferMessage to a specific output buffer message
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
/*
 * A message type encapsulating a client request for subscription to specific message types
 * #1 types of traffic desired
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
    std::vector<SerializableMessageType>    m_trafficTypes;
};

//==============================================================================
/*
 * A message type encapsulating control data from a client
 * #1 input mutes
 * #2 output mutes
 * #3 crosspoint states
 * #4 crosspoint values
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
            blob.copyTo(&inputMuteState, readPos, sizeof(inputMuteState));
            readPos += sizeof(inputMuteState);

            m_inputMuteStates[inputMuteState.first] = inputMuteState.second;
        }

        std::uint16_t outputMuteStatesCount;
        blob.copyTo(&outputMuteStatesCount, readPos, sizeof(std::uint16_t));
        readPos += sizeof(outputMuteStatesCount);
        for (int i = 0; i < outputMuteStatesCount; i++)
        {
            std::pair<std::uint16_t, bool> outputMuteState;
            blob.copyTo(&outputMuteState, readPos, sizeof(outputMuteState));
            readPos += sizeof(outputMuteState);

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

    const std::map<std::uint16_t, bool>& getInputMuteStates() const { return m_inputMuteStates; };
    const std::map<std::uint16_t, bool>& getOutputMuteStates() const { return m_outputMuteStates; };
    const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& getCrosspointStates() const { return m_crosspointStates; };
    const std::map<std::uint16_t, std::map<std::uint16_t, float>>& getCrosspointValues() const { return m_crosspointValues; };

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;

        auto inputMuteStatesCount = std::uint16_t(m_inputMuteStates.size());
        blob.append(&inputMuteStatesCount, sizeof(inputMuteStatesCount));
        for (auto& inputMuteStateKV : m_inputMuteStates)
            blob.append(&inputMuteStateKV, sizeof(inputMuteStateKV));

        auto outputMuteStatesCount = std::uint16_t(m_outputMuteStates.size());
        blob.append(&outputMuteStatesCount, sizeof(outputMuteStatesCount));
        for (auto& outputMuteStateKV : m_outputMuteStates)
            blob.append(&outputMuteStateKV, sizeof(outputMuteStateKV));

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
    std::map<std::uint16_t, bool>                           m_inputMuteStates;
    std::map<std::uint16_t, bool>                           m_outputMuteStates;
    std::map<std::uint16_t, std::map<std::uint16_t, bool>>  m_crosspointStates;
    std::map<std::uint16_t, std::map<std::uint16_t, float>> m_crosspointValues;
};

//==============================================================================
/*
 * A message type encapsulating plugin parameter information
 * Contains all metadata and current state of plugin parameters
 */
class PluginParameterInfosMessage : public SerializableMessage
{
public:
    PluginParameterInfosMessage() = default;
    PluginParameterInfosMessage(const std::vector<PluginParameterInfo>& parameterInfos)
    {
        m_type = SerializableMessageType::PluginParameterInfos;
        m_parameterInfos = parameterInfos;
    }

    PluginParameterInfosMessage(const juce::MemoryBlock& blob)
    {
        jassert(SerializableMessageType::PluginParameterInfos == static_cast<SerializableMessageType>(blob[0]));

        m_type = SerializableMessageType::PluginParameterInfos;

        auto readPos = int(sizeof(SerializableMessageType));

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

            m_parameterInfos.push_back(info);
        }
    }

    ~PluginParameterInfosMessage() = default;

    const std::vector<PluginParameterInfo>& getParameterInfos() const { return m_parameterInfos; }

protected:
    juce::MemoryBlock createSerializedContent(size_t& contentSize) const override
    {
        juce::MemoryBlock blob;

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
        }

        contentSize = blob.getSize();
        return blob;
    }

private:
    std::vector<PluginParameterInfo> m_parameterInfos;
};

//==============================================================================
/*
 * A message type encapsulating a single plugin parameter value change
 * Used for remote control and synchronization of individual parameter updates
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

    std::uint16_t getParameterIndex() const { return m_parameterIndex; }
    const juce::String& getParameterId() const { return m_parameterId; }
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
    std::uint16_t m_parameterIndex = 0;
    juce::String m_parameterId;
    float m_currentValue = 0.0f;
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

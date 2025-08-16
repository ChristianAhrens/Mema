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


namespace Mema
{


//==============================================================================
/*
 *
 */
class InputPositionMapper
{
public:
    //==============================================================================
    InputPositionMapper() = default;
    virtual ~InputPositionMapper() = default;

    void setOutputIncludePositions(const juce::Array<juce::AudioChannelSet::ChannelType>& outputIncludePositions)
    {
        if (m_outputIncludePositions != outputIncludePositions)
            m_outputIncludePositions = outputIncludePositions;
    };
    void setOutputIgnorePositions(const juce::Array<juce::AudioChannelSet::ChannelType>& outputIgnorePositions)
    {
        if (m_outputIgnorePositions != outputIgnorePositions)
            m_outputIgnorePositions = outputIgnorePositions;
    };

    void mapInputPosition(std::uint16_t channel, const juce::Point<float>& inputPosition, float sharpness)
    {
        jassert((onInputPositionMapped && getAngleForChannelType));
        if (onInputPositionMapped && getAngleForChannelType) // no need to do any processing if the mandatory access hooks are not set
        {
            std::map<juce::AudioChannelSet::ChannelType, juce::Point<float>> outputsMaxPoints;
            std::map<juce::AudioChannelSet::ChannelType, float> channelToOutputsDists;

            for (auto const& channelType : m_outputIncludePositions)
            {
                auto angleRad = juce::degreesToRadians(getAngleForChannelType(channelType));
                auto xLength = sinf(angleRad);
                auto yLength = cosf(angleRad);
                outputsMaxPoints[channelType] = juce::Point<float>(xLength, -yLength);

                // this is the actual primitive sourceposition-to-output level calculation algorithm
                auto outputMaxPoint = outputsMaxPoints[channelType];
                auto distance = outputMaxPoint.getDistanceFrom(inputPosition);
                auto base = 0.5f * distance;
                auto exp = jmap(sharpness, 1.0f, 5.0f);
                channelToOutputsDists[channelType] = powf(base, exp);

                //DBG(juce::String(__FUNCTION__) << " incl. " << juce::AudioChannelSet::getAbbreviatedChannelTypeName(channelType) << ": " << channelToOutputsDists[channelType]);
            }
            for (auto const& channelType : m_outputIgnorePositions)
            {
                channelToOutputsDists[channelType] = 0.0f;
                
                //DBG(juce::String(__FUNCTION__) << " excl. " << juce::AudioChannelSet::getAbbreviatedChannelTypeName(channelType) << ": " << channelToOutputsDists[channelType]);
            }

            onInputPositionMapped(channel, channelToOutputsDists);
        }
    }

    std::function<void(std::uint16_t, const std::map<juce::AudioChannelSet::ChannelType, float>&)>    onInputPositionMapped;
    std::function<float(const juce::AudioChannelSet::ChannelType&)> getAngleForChannelType;

protected:
    //==============================================================================

private:
    //==============================================================================

    //==============================================================================
    juce::Array<juce::AudioChannelSet::ChannelType> m_outputIncludePositions;
    juce::Array<juce::AudioChannelSet::ChannelType> m_outputIgnorePositions;
};


};

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


/** @class InputPositionMapper @brief Maps input-channel 2-D positions to per-output level (gain) values, based on proximity to configured output speaker positions. */
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
            std::map<juce::AudioChannelSet::ChannelType, float> channelToOutputsProximity;
            std::map<juce::AudioChannelSet::ChannelType, float> channelToOutputsDists;

            // first pass: determine each active output's fixed position on the panning circle and its
            // proximity to the current input position (1 = input sits exactly on the output, 0 = input
            // sits at the diametrically opposite point of the circle), and track the single nearest output.
            auto nearestChannelType = juce::AudioChannelSet::ChannelType::unknown;
            auto nearestProximity = -1.0f;
            for (auto const& channelType : m_outputIncludePositions)
            {
                auto angleRad = juce::degreesToRadians(getAngleForChannelType(channelType));
                auto xLength = sinf(angleRad);
                auto yLength = cosf(angleRad);
                outputsMaxPoints[channelType] = juce::Point<float>(xLength, -yLength);

                auto outputMaxPoint = outputsMaxPoints[channelType];
                auto distance = outputMaxPoint.getDistanceFrom(inputPosition);
                auto proximity = 1.0f - jlimit(0.0f, 1.0f, 0.5f * distance);
                channelToOutputsProximity[channelType] = proximity;

                if (proximity > nearestProximity)
                {
                    nearestProximity = proximity;
                    nearestChannelType = channelType;
                }
            }

            // second pass: this is the actual primitive sourceposition-to-output level calculation algorithm.
            // sharpness blends between the smooth falloff curve and a hard "nearest output only" cutoff,
            // with the crossfade confined to the top s_hardPanBlendRange of the sharpness scale so that
            // dragging sharpness up to its maximum does not produce an audible jump. That blend is further
            // gated by how far out the input position sits (radius 0 = field centre, 1 = the circle's
            // outer edge): single-output isolation is only ever reached exactly on the edge - moving the
            // position back towards the centre feeds neighbouring outputs increasingly again, even at
            // maximum sharpness. sharpness == 1.0 at radius == 1.0 still guarantees an exact single-output
            // result, since no finite exponent of the smooth curve alone can reach an exact zero.
            auto exp = jmap(sharpness, 1.0f, 5.0f);
            auto radius = jlimit(0.0f, 1.0f, inputPosition.getDistanceFromOrigin());
            auto hardPanBlend = jlimit(0.0f, 1.0f, (sharpness - (1.0f - s_hardPanBlendRange)) / s_hardPanBlendRange) * radius;
            for (auto const& channelType : m_outputIncludePositions)
            {
                auto smoothLevel = powf(channelToOutputsProximity[channelType], exp);
                auto hardLevel = (channelType == nearestChannelType) ? 1.0f : 0.0f;
                channelToOutputsDists[channelType] = jmap(hardPanBlend, smoothLevel, hardLevel);

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
    // portion of the sharpness range (immediately below its maximum of 1.0) over which
    // mapInputPosition crossfades from the smooth falloff curve to the hard single-output cutoff,
    // once the input position is also out at the panning circle's edge (see mapInputPosition).
    static constexpr float s_hardPanBlendRange = 0.1f;

    //==============================================================================
    juce::Array<juce::AudioChannelSet::ChannelType> m_outputIncludePositions;
    juce::Array<juce::AudioChannelSet::ChannelType> m_outputIgnorePositions;
};


};

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

#include "../MemaProcessorEditor/AbstractAudioVisualizer.h"

namespace Mema
{


//==============================================================================
/*
*/
class TwoDFieldMultisliderComponent  :   public juce::Component
{
public:
    enum ChannelLayer
    {
        Positioned,
        PositionedHeight,
        Directionless
    };

    struct TwoDMultisliderValue
    {
        TwoDMultisliderValue() = default;
        TwoDMultisliderValue(float rX, float rY) { relXPos = rX; relYPos = rY; };

        float relXPos = 0.0f;
        float relYPos = 0.0f;
    };

    struct TwoDMultisliderSourcePosition
    {
        TwoDMultisliderSourcePosition() = default;
        TwoDMultisliderSourcePosition(ChannelLayer l, TwoDMultisliderValue v, bool iS, bool iO) { layer = l; value = v; isSliding = iS; isOn = iO; };

        ChannelLayer layer = ChannelLayer::Positioned;
        TwoDMultisliderValue value = { 0.0f, 0.0f };
        bool isSliding = false;
        bool isOn = true;
    };

public:
    TwoDFieldMultisliderComponent();
    ~TwoDFieldMultisliderComponent();

    bool setChannelConfiguration(const juce::AudioChannelSet& channelLayout);
    const juce::Array<juce::AudioChannelSet>& getSupportedChannelConfigurations();

    float getAngleForChannelTypeInCurrentConfiguration(const juce::AudioChannelSet::ChannelType& channelType);
    int getChannelNumberForChannelTypeInCurrentConfiguration(const juce::AudioChannelSet::ChannelType& channelType);
    const juce::AudioChannelSet::ChannelType getChannelTypeForChannelNumberInCurrentConfiguration(int channelNumber);
    void setClockwiseOrderedChannelTypesForCurrentConfiguration();

    float getRequiredAspectRatio();

    void setIOCount(const std::pair<int, int>& ioCount);
    
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;

    //==============================================================================
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    
    //==============================================================================
    void setInputToOutputStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& inputToOutputStates);
    void setInputToOutputLevels(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& inputToOutputLevels);

    //==============================================================================
    void setInputPosition(std::uint16_t channel, const TwoDMultisliderValue& value, const ChannelLayer& layer, juce::NotificationType notification = juce::dontSendNotification);
    void selectInput(std::uint16_t channel, bool selectOn, juce::NotificationType notification = juce::dontSendNotification);

    //==============================================================================
    const juce::Array<juce::AudioChannelSet::ChannelType>& getOutputsInLayer(const ChannelLayer& layer);
    const juce::Array<juce::AudioChannelSet::ChannelType> getDirectiveOutputsNotInLayer(const ChannelLayer& layer);
    
    //==============================================================================
    std::function<void(std::uint16_t channel, const TwoDMultisliderValue& value, std::optional<ChannelLayer> layer)> onInputPositionChanged;
    std::function<void(std::uint16_t channel)> onInputSelected;
    std::function<void(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>&)>  onInputToOutputStatesChanged;
    std::function<void(const std::map<std::uint16_t, std::map<std::uint16_t, float>>&)> onInputToOutputValuesChanged;

    //==============================================================================
    static constexpr int s_thumbWidth = 20;
    static constexpr float s_trackWidth = 8.0f;

private:
    //==============================================================================
    void paintCircularLevelIndication(juce::Graphics& g, const juce::Rectangle<float>& circleArea, const std::map<int, juce::Point<float>>& channelLevelMaxPoints, const juce::Array<juce::AudioChannelSet::ChannelType>& channelsToPaint);
    void paintSliderKnob(juce::Graphics& g, const juce::Rectangle<float>& sliderArea, const float& relXPos, const float& relYPos, const int& silderNumber, bool isSliderOn, bool isSliderSliding);

    //==============================================================================
    bool usesPositionedChannels() { return !m_clockwiseOrderedChannelTypes.isEmpty(); };
    bool usesPositionedHeightChannels() { return !m_clockwiseOrderedHeightChannelTypes.isEmpty(); };
    bool usesDirectionlessChannels() { return !m_directionLessChannelTypes.isEmpty(); };

    //==============================================================================
    void rebuildDirectionslessChannelSliders();
    void configureDirectionlessSliderToRelativeCtrl(const juce::AudioChannelSet::ChannelType& channelType, JUCEAppBasics::ToggleStateSlider& slider);
    
    //==============================================================================
    juce::Rectangle<float>  m_positionedChannelsArea;
    juce::Rectangle<float>  m_positionedHeightChannelsArea;
    juce::Rectangle<float>  m_directionlessChannelsArea;

    std::map<int, juce::Point<float>>   m_channelLevelMaxPoints;
    std::map<int, juce::Point<float>>   m_channelHeightLevelMaxPoints;

    juce::AudioChannelSet                           m_channelConfiguration;
    juce::Array<juce::AudioChannelSet::ChannelType> m_clockwiseOrderedChannelTypes;
    juce::Array<juce::AudioChannelSet::ChannelType> m_clockwiseOrderedHeightChannelTypes;
    juce::Array<juce::AudioChannelSet::ChannelType> m_directionLessChannelTypes;
    juce::Array<juce::AudioChannelSet>              m_supportedChannelConfigurations = { 
        juce::AudioChannelSet::mono(),
        juce::AudioChannelSet::stereo(),
        juce::AudioChannelSet::createLCR(),
        juce::AudioChannelSet::createLCRS(),
        juce::AudioChannelSet::createLRS(),
        juce::AudioChannelSet::create5point0(),
        juce::AudioChannelSet::create5point1(),
        juce::AudioChannelSet::create5point1point2(),
        juce::AudioChannelSet::create7point0(),
        juce::AudioChannelSet::create7point1(),
        juce::AudioChannelSet::create7point1point4(),
        juce::AudioChannelSet::create9point1point6() };

    std::map<juce::AudioChannelSet::ChannelType, std::unique_ptr<JUCEAppBasics::ToggleStateSlider>> m_directionslessChannelSliders;
    std::map<juce::AudioChannelSet::ChannelType, std::unique_ptr<juce::Label>>                      m_directionslessChannelLabels;
    std::map<juce::AudioChannelSet::ChannelType, double>                                            m_directionlessSliderRelRef;

    std::map<std::uint16_t, std::map<juce::AudioChannelSet::ChannelType, std::pair<bool, float>>> m_inputToOutputVals;
    std::map<std::uint16_t, TwoDMultisliderSourcePosition>  m_inputPositions;

    int m_currentOutputCount = 0;

    std::uint16_t m_currentlySelectedInput = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwoDFieldMultisliderComponent)
};

}
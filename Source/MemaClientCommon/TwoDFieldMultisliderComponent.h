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

#include "TwoDFieldBase.h"
#include "../MemaProcessorEditor/AbstractAudioVisualizer.h"

 /**
  * Fwd. Decls.
  */
namespace JUCEAppBasics
{
    class FixedFontTextEditor;
}

namespace Mema
{


//==============================================================================
/*
*/
class TwoDFieldMultisliderComponent  :   public TwoDFieldBase, public juce::Component
{
public:
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
        TwoDMultisliderSourcePosition(ChannelLayer l, TwoDMultisliderValue v, float s, bool iS, bool iO) { layer = l; value = v; sharpness = s; isSliding = iS; isOn = iO; };

        ChannelLayer layer = ChannelLayer::Positioned;
        TwoDMultisliderValue value = { 0.0f, 0.0f };
        float sharpness = 0.4f;
        bool isSliding = false;
        bool isOn = true;
    };

public:
    TwoDFieldMultisliderComponent();
    ~TwoDFieldMultisliderComponent();

    bool setChannelConfiguration(const juce::AudioChannelSet& channelLayout) override;

    void setIOCount(const std::pair<int, int>& ioCount);

    void setControlsSize(int ctrlsSize);
    
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;
    std::unique_ptr<juce::ComponentTraverser> createKeyboardFocusTraverser() override;

    //==============================================================================
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    
    //==============================================================================
    void setInputToOutputStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& inputToOutputStates);
    void setInputToOutputLevels(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& inputToOutputLevels);

    //==============================================================================
    void setInputPosition(std::uint16_t channel, const TwoDMultisliderValue& value, const float& panningSharpness, const ChannelLayer& layer, juce::NotificationType notification = juce::dontSendNotification);
    void setInputPositionValue(std::uint16_t channel, const TwoDMultisliderValue& value, juce::NotificationType notification = juce::dontSendNotification);
    void setInputPositionSharpness(std::uint16_t channel, const float& sharpness, juce::NotificationType notification = juce::dontSendNotification);
    void setInputPositionLayer(std::uint16_t channel, const ChannelLayer& layer, juce::NotificationType notification = juce::dontSendNotification);
    void selectInput(std::uint16_t channel, bool selectOn, juce::NotificationType notification = juce::dontSendNotification);

    void triggerInputPositionsDump();

    //==============================================================================
    std::function<void(std::uint16_t channel, const TwoDMultisliderValue& value, const float& sharpness, std::optional<ChannelLayer> layer)> onInputPositionChanged;
    std::function<void(std::uint16_t channel)> onInputSelected;
    std::function<void(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>&)>  onInputToOutputStatesChanged;
    std::function<void(const std::map<std::uint16_t, std::map<std::uint16_t, float>>&)> onInputToOutputValuesChanged;

private:
    //==============================================================================
    void paintCircularLevelIndication(juce::Graphics& g, const juce::Rectangle<float>& circleArea, const std::map<int, juce::Point<float>>& channelLevelMaxPoints, const juce::Array<juce::AudioChannelSet::ChannelType>& channelsToPaint);
    void paintSliderKnob(juce::Graphics& g, const juce::Rectangle<float>& sliderArea, const float& relXPos, const float& relYPos, const int& silderNumber, bool isSliderOn, bool isSliderSliding);

    //==============================================================================
    void rebuildDirectionslessChannelSliders();
    void configureDirectionlessSliderToRelativeCtrl(const juce::AudioChannelSet::ChannelType& channelType, JUCEAppBasics::ToggleStateSlider& slider);
    
    //==============================================================================
    std::map<juce::AudioChannelSet::ChannelType, std::unique_ptr<JUCEAppBasics::ToggleStateSlider>> m_directionslessChannelSliders;
    std::map<juce::AudioChannelSet::ChannelType, std::unique_ptr<juce::Label>>                      m_directionslessChannelLabels;
    std::map<juce::AudioChannelSet::ChannelType, double>                                            m_directionlessSliderRelRef;

    std::map<std::uint16_t, std::map<juce::AudioChannelSet::ChannelType, std::pair<bool, float>>> m_inputToOutputVals;
    
    std::map<std::uint16_t, TwoDMultisliderSourcePosition>  m_inputPositions;
    std::vector<std::uint16_t>                              m_inputPositionStackingOrder;
    std::uint16_t                                           m_currentlySelectedInput = 0;

    std::unique_ptr<juce::Label>                            m_sharpnessLabel;
    std::unique_ptr<JUCEAppBasics::FixedFontTextEditor>     m_sharpnessEdit;

    int m_currentOutputCount = 0;

    //==============================================================================
    int m_thumbWidth = 0;
    float m_trackWidth = 0.0f;
    int m_ctrlsSize = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwoDFieldMultisliderComponent)
};

}
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

#include "TwoDFieldMultisliderComponent.h"

#include <CustomLookAndFeel.h>


namespace Mema
{

//#define PAINTINGHELPER

//==============================================================================
TwoDFieldMultisliderComponent::TwoDFieldMultisliderComponent()
    :   juce::Component()
{
    //setUsesValuesInDB(true);
}

TwoDFieldMultisliderComponent::~TwoDFieldMultisliderComponent()
{
}

void TwoDFieldMultisliderComponent::paint (juce::Graphics& g)
{
    //AbstractAudioVisualizer::paint(g);

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // paint the level indications where applicable
    if (!m_positionedChannelsArea.isEmpty())
        paintCircularLevelIndication(g, m_positionedChannelsArea, m_channelLevelMaxPoints, m_clockwiseOrderedChannelTypes);
    if (!m_positionedHeightChannelsArea.isEmpty())
        paintCircularLevelIndication(g, m_positionedHeightChannelsArea, m_channelHeightLevelMaxPoints, m_clockwiseOrderedHeightChannelTypes);

    // paint slider knobs
    for (auto const& inputPosition : m_inputPositions)
    {
        auto emptyRect = juce::Rectangle<float>();
        auto& area = emptyRect;
        if (ChannelLayer::Positioned == inputPosition.second.layer && !m_positionedChannelsArea.isEmpty())
            area = m_positionedChannelsArea;
        else if (ChannelLayer::PositionedHeight == inputPosition.second.layer && !m_positionedHeightChannelsArea.isEmpty())
            area = m_positionedHeightChannelsArea;

        if (!area.isEmpty())
            paintSliderKnob(g, area, inputPosition.second.value.relXPos, inputPosition.second.value.relYPos, inputPosition.first, inputPosition.second.isOn, inputPosition.second.isSliding);
    }

    //// draw dBFS
    //g.setFont(12.0f);
    //g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOffId));
    //juce::String rangeText;
    ////if (getUsesValuesInDB())
    ////    rangeText = juce::String(ProcessorDataAnalyzer::getGlobalMindB()) + " ... " + juce::String(ProcessorDataAnalyzer::getGlobalMaxdB()) + " dBFS";
    ////else
    //    rangeText = "0 ... 1";
    //g.drawText(rangeText, getLocalBounds().removeFromLeft(getLocalBounds().getWidth() - m_directionlessChannelsArea.toNearestInt().getWidth()), juce::Justification::topRight, true);
}

void TwoDFieldMultisliderComponent::paintCircularLevelIndication(juce::Graphics& g, const juce::Rectangle<float>& circleArea, const std::map<int, juce::Point<float>>& channelLevelMaxPoints, const juce::Array<juce::AudioChannelSet::ChannelType>& channelsToPaint)
{
#if defined DEBUG && defined PAINTINGHELPER
    g.setColour(juce::Colours::blueviolet);
    g.drawRect(circleArea);
#endif

    // fill circle background
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker());
    g.fillEllipse(circleArea);

#if defined DEBUG && defined PAINTINGHELPER
    g.setColour(juce::Colours::red);
    g.drawRect(circleArea);
    g.setColour(juce::Colours::blue);
    g.drawRect(getLocalBounds());
#endif


    const float meterWidth = 5.0f;
    const float halfMeterWidth = 2.0f;


    auto circleCenter = circleArea.getCentre();

    // prepare max points
    std::map<int, juce::Point<float>> centerToMaxVectors;
    std::map<int, juce::Point<float>> meterWidthOffsetVectors;
    for (int i = 0; i < channelsToPaint.size(); i++)
    {
        auto const& channelType = channelsToPaint[i];
        auto iTy = int(channelType);
        if (0 < channelLevelMaxPoints.count(iTy))
        {
            auto angleRad = juce::degreesToRadians(getAngleForChannelTypeInCurrentConfiguration(channelType));
            centerToMaxVectors[iTy] = circleCenter - channelLevelMaxPoints.at(iTy);
            meterWidthOffsetVectors[iTy] = { cosf(angleRad) * halfMeterWidth, sinf(angleRad) * halfMeterWidth };
        }
    }

    // helper std::functions to avoid codeclones below
    auto createAndPaintLevelPath = [=](std::map<int, juce::Point<float>>& centerToMaxPoints, std::map<int, juce::Point<float>>& meterWidthOffsetPoints, std::map<juce::AudioChannelSet::ChannelType, float>& levels, juce::Graphics& g, const juce::Colour& colour, bool stroke) {
        juce::Path path;
        auto pathStarted = false;
        for (auto const& channelType : channelsToPaint)
        {
            auto iTy = int(channelType);
            auto channelMaxPoint = circleCenter - (centerToMaxPoints[iTy] * levels[channelType]);

            if (!pathStarted)
            {
                path.startNewSubPath(channelMaxPoint - meterWidthOffsetPoints[iTy]);
                pathStarted = true;
            }
            else
                path.lineTo(channelMaxPoint - meterWidthOffsetPoints[iTy]);

            path.lineTo(channelMaxPoint + meterWidthOffsetPoints[iTy]);
        }
        path.closeSubPath();

        g.setColour(colour);
        if (stroke)
            g.strokePath(path, juce::PathStrokeType(1));
        else
            g.fillPath(path);
#if defined DEBUG && defined PAINTINGHELPER
        g.setColour(juce::Colours::yellow);
        g.drawRect(path.getBounds());
#endif
    };
    auto paintLevelMeterLines = [=](std::map<int, juce::Point<float>>& centerToMaxPoints, std::map<int, juce::Point<float>>& meterWidthOffsetPoints, std::map<juce::AudioChannelSet::ChannelType, float>& levels, juce::Graphics& g, const juce::Colour& colour, bool isHoldLine) {
        g.setColour(colour);
        for (auto const& channelType : channelsToPaint)
        {
            auto iTy = int(channelType);
            auto channelMaxPoint = circleCenter - (centerToMaxPoints[iTy] * levels[channelType]);

            if (isHoldLine)
                g.drawLine(juce::Line<float>(channelMaxPoint - meterWidthOffsetPoints[iTy], channelMaxPoint + meterWidthOffsetPoints[iTy]), 1.0f);
            else
                g.drawLine(juce::Line<float>(circleCenter, channelMaxPoint), meterWidth);
        }
        };
    
    std::map<juce::AudioChannelSet::ChannelType, float> levels;
    for (auto& vKV : m_inputToOutputVals)
    {
        levels.clear();
        for (auto const& oVals : vKV.second)
        {
            auto& channel = oVals.first;
            auto& state = oVals.second.first;
            auto& level = oVals.second.second;
            levels[channel] = state ? level : 0.0f;
        }

        juce::Colour colour;
        if (m_currentlySelectedInput == vKV.first)
            colour = getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId).withAlpha(0.8f);
        else
            colour = getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId).withAlpha(0.4f);

        // paint hold values as path
        createAndPaintLevelPath(centerToMaxVectors, meterWidthOffsetVectors, levels, g, colour, false);
        // paint hold values as max line
        paintLevelMeterLines(centerToMaxVectors, meterWidthOffsetVectors, levels, g, colour, false);
    }

    // draw a simple circle surrounding
    g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOffId));
    g.drawEllipse(circleArea, 1);

    // draw dashed field dimension indication lines
    float dparam[]{ 4.0f, 5.0f };
    for (auto const& channelType : channelsToPaint)
        if (0 < channelLevelMaxPoints.count(channelType))
            g.drawDashedLine(juce::Line<float>(channelLevelMaxPoints.at(channelType), circleCenter), dparam, 2);

    // draw channelType naming legend
    g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOffId));
    for (auto const& channelType : channelsToPaint)
    {
        if (0 >= channelLevelMaxPoints.count(channelType))
            continue;

        auto channelName = juce::AudioChannelSet::getAbbreviatedChannelTypeName(channelType);
        auto textRect = juce::Rectangle<float>(juce::GlyphArrangement::getStringWidth(g.getCurrentFont(), channelName), g.getCurrentFont().getHeight());
        auto angle = getAngleForChannelTypeInCurrentConfiguration(channelType);
        auto textRectOffset = juce::Point<int>(-int(textRect.getWidth() / 2.0f), 0);
        if (90.0f < angle)
            angle += 180.0f;
        else if (-90.0f > angle)
            angle -= 180.0f;
        else
            textRectOffset.addXY(0, -int(g.getCurrentFont().getHeight()));
        auto angleRad = juce::degreesToRadians(angle);

        g.saveState();
        g.setOrigin(channelLevelMaxPoints.at(channelType).toInt());
        g.addTransform(juce::AffineTransform().translated(textRectOffset).rotated(angleRad));
        g.drawText(channelName, textRect, Justification::centred, true);

#if defined DEBUG && defined PAINTINGHELPER
        g.setColour(juce::Colours::lightblue);
        g.drawRect(textRect);
        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOffId));
#endif

        g.restoreState();
    }
}

void TwoDFieldMultisliderComponent::paintSliderKnob(juce::Graphics& g, const juce::Rectangle<float>& sliderArea, const float& relXPos, const float& relYPos, const int& silderNumber, bool isSliderOn, bool isSliderSliding)
{
    juce::Path valueTrack;
    auto minPoint = sliderArea.getCentre();
    auto maxPoint = sliderArea.getCentre() - juce::Point<float>((sliderArea.getWidth() / 2) * relXPos, (sliderArea.getHeight() / 2) * relYPos);

    if (isSliderOn)
    {
        if (isSliderSliding)
        {
            valueTrack.startNewSubPath(minPoint);
            valueTrack.lineTo(maxPoint);
            g.setColour(getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringPeakColourId));
            g.strokePath(valueTrack, { s_trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded });
        }

        g.setColour(getLookAndFeel().findColour(juce::Slider::thumbColourId));
        g.fillEllipse(juce::Rectangle<float>(static_cast<float>(s_thumbWidth), static_cast<float>(s_thumbWidth)).withCentre(maxPoint));

        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOnId));
        g.drawText(juce::String(silderNumber), juce::Rectangle<float>(static_cast<float>(s_thumbWidth), static_cast<float>(s_thumbWidth)).withCentre(maxPoint), juce::Justification::centred);
    }
    else
    {
        if (isSliderSliding)
        {
            valueTrack.startNewSubPath(minPoint);
            valueTrack.lineTo(maxPoint);
            auto valueTrackOutline = valueTrack;
            juce::PathStrokeType pt(s_trackWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
            pt.createStrokedPath(valueTrackOutline, valueTrack);
            g.setColour(getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringPeakColourId));
            g.strokePath(valueTrackOutline, juce::PathStrokeType(1.0f));
        }

        g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::ColourIds::backgroundColourId));
        g.fillEllipse(juce::Rectangle<float>(static_cast<float>(s_thumbWidth - 1), static_cast<float>(s_thumbWidth - 1)).withCentre(maxPoint));

        g.setColour(getLookAndFeel().findColour(juce::Slider::thumbColourId));
        g.drawEllipse(juce::Rectangle<float>(static_cast<float>(s_thumbWidth), static_cast<float>(s_thumbWidth)).withCentre(maxPoint), 1.0f);

        g.drawText(juce::String(silderNumber), juce::Rectangle<float>(static_cast<float> (s_thumbWidth), static_cast<float> (s_thumbWidth)).withCentre(maxPoint), juce::Justification::centred);
    }
}

void TwoDFieldMultisliderComponent::resized()
{
    // process areas for level indication painting
    auto coreTwoDFieldOnly = usesPositionedChannels() && !usesPositionedHeightChannels() && !usesDirectionlessChannels();
    auto coreTwoDFieldWithMeterbridge = usesPositionedChannels() && !usesPositionedHeightChannels() && usesDirectionlessChannels();
    auto bothTwoDFields = usesPositionedChannels() && usesPositionedHeightChannels() && !usesDirectionlessChannels();
    auto bothTwoDFieldsWithMeterbridge = usesPositionedChannels() && usesPositionedHeightChannels() && usesDirectionlessChannels();

    auto margin = 12.0f;
    auto bounds = getLocalBounds().toFloat();
    auto width = bounds.getWidth();
    auto height = bounds.getHeight();
    if (coreTwoDFieldOnly)
    {
        m_positionedChannelsArea = bounds.reduced(margin);
        m_positionedHeightChannelsArea = {};
        m_directionlessChannelsArea = {};
    }
    else if (coreTwoDFieldWithMeterbridge)
    {
        m_positionedChannelsArea = bounds.reduced(margin);
        m_positionedChannelsArea.removeFromRight(width * (1.0f / 11.0f));

        m_positionedHeightChannelsArea = {};

        m_directionlessChannelsArea = bounds;
        m_directionlessChannelsArea.removeFromLeft(width * (10.0f / 11.0f));
    }
    else if (bothTwoDFields)
    {
        m_positionedHeightChannelsArea = bounds.reduced(margin);
        m_positionedHeightChannelsArea.removeFromRight(width * (8.4f / 12.0f));
        m_positionedHeightChannelsArea.removeFromBottom(height * (5.4f / 10.0f));

        m_positionedChannelsArea = bounds.reduced(margin);
        m_positionedChannelsArea.removeFromLeft(width * (3.4f / 12.0f));
        m_positionedChannelsArea.removeFromTop(height * (1.4f / 10.0f));

        m_directionlessChannelsArea = {};
    }
    else if (bothTwoDFieldsWithMeterbridge)
    {
        m_positionedHeightChannelsArea = bounds.reduced(margin);
        m_positionedHeightChannelsArea.removeFromRight(width * (8.4f / 13.0f));
        m_positionedHeightChannelsArea.removeFromBottom(height * (5.4f / 10.0f));
        
        m_positionedChannelsArea = bounds;
        m_directionlessChannelsArea = m_positionedChannelsArea.removeFromRight(width * (1.0f / 13.0f));
        m_positionedChannelsArea.reduce(margin, margin);
        m_positionedChannelsArea.removeFromLeft(width * (3.4f / 13.0f));
        m_positionedChannelsArea.removeFromTop(height * (1.4f / 10.0f));
    }

    for (auto const& channelType : m_clockwiseOrderedChannelTypes)
    {
        auto angleRad = juce::degreesToRadians(getAngleForChannelTypeInCurrentConfiguration(channelType));
        auto xLength = sinf(angleRad) * (m_positionedChannelsArea.getHeight() / 2);
        auto yLength = cosf(angleRad) * (m_positionedChannelsArea.getWidth() / 2);
        m_channelLevelMaxPoints[channelType] = m_positionedChannelsArea.getCentre() + juce::Point<float>(xLength, -yLength);
    }

    for (auto const& channelType : m_clockwiseOrderedHeightChannelTypes)
    {
        auto angleRad = juce::degreesToRadians(getAngleForChannelTypeInCurrentConfiguration(channelType));
        auto xLength = sinf(angleRad) * (m_positionedHeightChannelsArea.getHeight() / 2);
        auto yLength = cosf(angleRad) * (m_positionedHeightChannelsArea.getWidth() / 2);
        m_channelHeightLevelMaxPoints[channelType] = m_positionedHeightChannelsArea.getCentre() + juce::Point<float>(xLength, -yLength);
    }

    if (!m_directionlessChannelsArea.isEmpty() && !m_directionslessChannelSliders.empty() && !m_directionslessChannelLabels.empty())
    {
        jassert(m_directionslessChannelSliders.size() == m_directionslessChannelLabels.size());
        auto directionlessChannelsWidth = m_directionlessChannelsArea.getWidth() / m_directionslessChannelSliders.size();
        auto areaToDivide = m_directionlessChannelsArea;
        for (auto const& sliderKV : m_directionslessChannelSliders)
        {
            auto const& slider = sliderKV.second;
            auto const& label = m_directionslessChannelLabels.at(sliderKV.first);
            auto sliderBounds = areaToDivide.removeFromLeft(directionlessChannelsWidth).toNearestInt();
            auto labelBounds = sliderBounds.removeFromBottom(s_thumbWidth);
            if (slider)
                slider->setBounds(sliderBounds);
            if (label)
                label->setBounds(labelBounds);
        }
    }
}

void TwoDFieldMultisliderComponent::lookAndFeelChanged()
{
    if (!m_directionslessChannelSliders.empty())
    {
        for (auto const& slider : m_directionslessChannelSliders)
            if (slider.second)
                slider.second->setColour(juce::Slider::ColourIds::trackColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    }
}

void TwoDFieldMultisliderComponent::mouseDown(const juce::MouseEvent& e)
{
    // hit-test slider knobs
    auto hadHit = false;
    for (auto& inputPosition : m_inputPositions)
    {
        auto emptyRect = juce::Rectangle<float>();
        auto& area = emptyRect;
        if (ChannelLayer::Positioned == inputPosition.second.layer && !m_positionedChannelsArea.isEmpty())
            area = m_positionedChannelsArea;
        else if (ChannelLayer::PositionedHeight == inputPosition.second.layer && !m_positionedHeightChannelsArea.isEmpty())
            area = m_positionedHeightChannelsArea;
        else if (ChannelLayer::Directionless == inputPosition.second.layer && !m_directionlessChannelsArea.isEmpty())
            area = m_directionlessChannelsArea;

        auto maxPoint = area.getCentre() - juce::Point<float>((area.getWidth() / 2) * inputPosition.second.value.relXPos, (area.getHeight() / 2) * inputPosition.second.value.relYPos);
        auto sliderKnob = juce::Rectangle<float>(static_cast<float>(s_thumbWidth), static_cast<float>(s_thumbWidth)).withCentre(maxPoint);
        if (sliderKnob.contains(e.getMouseDownPosition().toFloat()) && false == hadHit)
        {
            inputPosition.second.isOn = true;

            selectInput(inputPosition.first, true, juce::sendNotification);

            hadHit = true;
        }
        else
        {
            selectInput(inputPosition.first, false, juce::sendNotification);
        }
    }

    juce::Component::mouseDown(e);
}

void TwoDFieldMultisliderComponent::mouseUp(const MouseEvent& e)
{
    juce::Component::mouseUp(e);
}

void TwoDFieldMultisliderComponent::mouseDrag(const MouseEvent& e)
{
    if (e.mouseWasDraggedSinceMouseDown())
    {
        // reset any sliding states slider knobs
        for (auto& inputPosition : m_inputPositions)
        {
            if (inputPosition.second.isSliding)
            {
                auto emptyRect = juce::Rectangle<float>();
                auto& area = emptyRect;
                if (ChannelLayer::Positioned == inputPosition.second.layer && !m_positionedChannelsArea.isEmpty())
                    area = m_positionedChannelsArea;
                else if (ChannelLayer::PositionedHeight == inputPosition.second.layer && !m_positionedHeightChannelsArea.isEmpty())
                    area = m_positionedHeightChannelsArea;
                else if (ChannelLayer::Directionless == inputPosition.second.layer && !m_directionlessChannelsArea.isEmpty())
                    area = m_directionlessChannelsArea;

                auto mousePosition = e.getMouseDownPosition() + e.getOffsetFromDragStart();
                
                juce::Path ellipsePath;
                ellipsePath.addEllipse(area);
                // if the mouse is within the resp. circle, do the regular calculation of knob pos
                if (ellipsePath.contains(mousePosition.toFloat()))
                {
                    auto positionInArea = area.getCentre() - area.getConstrainedPoint(mousePosition.toFloat());
                    auto relXPos = positionInArea.getX() / (0.5f * area.getWidth());
                    auto relYPos = positionInArea.getY() / (0.5f * area.getHeight());
                    setInputPosition(inputPosition.first, { relXPos, relYPos }, inputPosition.second.layer, juce::sendNotification);
                }
                else
                {
                    juce::Path positionedChannelsEllipsePath, positionedHeightChannelsEllipsePath;
                    positionedChannelsEllipsePath.addEllipse(m_positionedChannelsArea);
                    positionedHeightChannelsEllipsePath.addEllipse(m_positionedHeightChannelsArea);
                    // check if the mouse has entered another circle area while dragging
                    if (positionedChannelsEllipsePath.contains(mousePosition.toFloat()))
                    {
                        auto positionInArea = m_positionedChannelsArea.getCentre() - m_positionedChannelsArea.getConstrainedPoint(mousePosition.toFloat());
                        auto relXPos = positionInArea.getX() / (0.5f * m_positionedChannelsArea.getWidth());
                        auto relYPos = positionInArea.getY() / (0.5f * m_positionedChannelsArea.getHeight());
                        setInputPosition(inputPosition.first, { relXPos, relYPos }, ChannelLayer::Positioned, juce::sendNotification);
                    }
                    else if (positionedHeightChannelsEllipsePath.contains(mousePosition.toFloat()))
                    {
                        auto positionInArea = m_positionedHeightChannelsArea.getCentre() - m_positionedHeightChannelsArea.getConstrainedPoint(mousePosition.toFloat());
                        auto relXPos = positionInArea.getX() / (0.5f * m_positionedHeightChannelsArea.getWidth());
                        auto relYPos = positionInArea.getY() / (0.5f * m_positionedHeightChannelsArea.getHeight());
                        setInputPosition(inputPosition.first, { relXPos, relYPos }, ChannelLayer::PositionedHeight, juce::sendNotification);
                    }
                    // finally do the clipping to original circle, if the dragging happens somewhere outside everything
                    else
                    {
                        juce::Point<float> constrainedPoint;
                        ellipsePath.getNearestPoint(mousePosition.toFloat(), constrainedPoint);
                        auto positionInArea = area.getCentre() - constrainedPoint;
                        auto relXPos = positionInArea.getX() / (0.5f * area.getWidth());
                        auto relYPos = positionInArea.getY() / (0.5f * area.getHeight());
                        inputPosition.second.value = { relXPos, relYPos };
                        setInputPosition(inputPosition.first, { relXPos, relYPos }, inputPosition.second.layer, juce::sendNotification);
                    }

                }
            }
        }
    }

    juce::Component::mouseDrag(e);
}

void TwoDFieldMultisliderComponent::setInputPosition(std::uint16_t channel, const TwoDMultisliderValue& value, const ChannelLayer& layer, juce::NotificationType notification)
{
    m_inputPositions[channel].value = value;
    m_inputPositions[channel].layer = layer;

    repaint();

    //DBG(juce::String(__FUNCTION__) << " new pos: " << int(channel) << " " << value.relXPos << "," << value.relYPos << "(" << layer << ")");

    if (juce::dontSendNotification != notification && onInputPositionChanged)
        onInputPositionChanged(channel, value, layer);
}

void TwoDFieldMultisliderComponent::selectInput(std::uint16_t channel, bool selectOn, juce::NotificationType notification)
{
    m_inputPositions[channel].isSliding = selectOn;
    if (m_currentlySelectedInput == channel && !selectOn)
        m_currentlySelectedInput = 0;
    else if (m_currentlySelectedInput != channel && selectOn)
        m_currentlySelectedInput = channel;

    repaint();

    if (!m_directionslessChannelSliders.empty() && !m_inputToOutputVals.empty())
    {
        for (auto const& slider : m_directionslessChannelSliders)
        {
            if (slider.second)
            {
                auto output = slider.first;
                jassert(0 < m_inputToOutputVals.count(channel));
                if (selectOn && 0 < m_inputToOutputVals.count(channel))
                {
                    jassert(0 < m_inputToOutputVals.at(channel).count(output));
                    if(0 < m_inputToOutputVals.at(channel).count(output))
                    {
                        slider.second->setTitle(juce::String(channel));
                        slider.second->setRange(0.0, 1.0, 0.01);
                        slider.second->setValue(m_inputToOutputVals.at(channel).at(output).second);
                        slider.second->setToggleState(m_inputToOutputVals.at(channel).at(output).first, juce::dontSendNotification);
                    }
                }
                else if (0 == m_currentlySelectedInput)
                {
                    slider.second->setTitle("");
                    slider.second->setRange(0.0, 1.0, 0.01);
                    slider.second->setValue(0.5); // relative starting
                    slider.second->setToggleState(true, juce::dontSendNotification);
                    m_directionlessSliderRelRef[slider.first] = 0.5;
                }
            }
        }
    }

    if (juce::dontSendNotification != notification && onInputSelected && selectOn)
        onInputSelected(channel);
}

void TwoDFieldMultisliderComponent::setIOCount(const std::pair<int, int>& ioCount)
{
    m_inputPositions.clear();
    for (auto i = 1, p = 50; i <= ioCount.first; i++, p-=5)
    {
        m_inputPositions[std::uint16_t(i)] = { ChannelLayer::Positioned, { 0.01f * float(p), 0.5f }, false, false };
    }

    repaint();
}

void TwoDFieldMultisliderComponent::setInputToOutputStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& inputToOutputStates)
{
    for (auto const& iKV : inputToOutputStates)
    {
        for (auto const& oKV : iKV.second)
        {
            jassert(0 < iKV.first);
            auto output = getChannelTypeForChannelNumberInCurrentConfiguration(oKV.first);
            if (juce::AudioChannelSet::ChannelType::unknown != output)
                m_inputToOutputVals[iKV.first][output].first = oKV.second;
            if (m_directionLessChannelTypes.contains(output) && m_directionslessChannelSliders.at(output) && m_currentlySelectedInput == iKV.first)
                m_directionslessChannelSliders.at(output)->setValue(oKV.second, juce::dontSendNotification);
        }
    }

    repaint();
}

void TwoDFieldMultisliderComponent::setInputToOutputLevels(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& inputToOutputLevels)
{
    for (auto const& iKV : inputToOutputLevels)
    {
        for (auto const& oKV : iKV.second)
        {
            jassert(0 < iKV.first);
            auto output = getChannelTypeForChannelNumberInCurrentConfiguration(oKV.first);
            if (juce::AudioChannelSet::ChannelType::unknown != output)
                m_inputToOutputVals[iKV.first][output].second = oKV.second;
            if (m_directionLessChannelTypes.contains(output) && m_directionslessChannelSliders.at(output) && m_currentlySelectedInput == iKV.first)
                m_directionslessChannelSliders.at(output)->setValue(oKV.second, juce::dontSendNotification);
        }
    }

    repaint();
}

bool TwoDFieldMultisliderComponent::setChannelConfiguration(const juce::AudioChannelSet& channelLayout)
{
    auto wasUpdated = false;
    if (getSupportedChannelConfigurations().contains(channelLayout))
    {
        if (m_channelConfiguration != channelLayout)
        {
            m_channelConfiguration = channelLayout;
            wasUpdated = true;
        }
    }
    else
        m_channelConfiguration = juce::AudioChannelSet::mono();

    if (wasUpdated)
    {
        setClockwiseOrderedChannelTypesForCurrentConfiguration();
        rebuildDirectionslessChannelSliders();
    }

    return wasUpdated;
}

const juce::Array<juce::AudioChannelSet>& TwoDFieldMultisliderComponent::getSupportedChannelConfigurations()
{
    return m_supportedChannelConfigurations;
}

float TwoDFieldMultisliderComponent::getAngleForChannelTypeInCurrentConfiguration(const juce::AudioChannelSet::ChannelType& channelType)
{
    if (juce::AudioChannelSet::mono() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::centre:
            return 0.0f;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::stereo() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return -30.0f;
        case juce::AudioChannelSet::ChannelType::right:
            return 30.0f;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::createLCR() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return -30.0f;
        case juce::AudioChannelSet::ChannelType::right:
            return 30.0f;
        case juce::AudioChannelSet::ChannelType::centre:
            return 0.0f;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::createLCRS() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return -30.0f;
        case juce::AudioChannelSet::ChannelType::right:
            return 30.0f;
        case juce::AudioChannelSet::ChannelType::centre:
            return 0.0f;
        case juce::AudioChannelSet::ChannelType::surround:
            return 180.0f;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::createLRS() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return -30.0f;
        case juce::AudioChannelSet::ChannelType::right:
            return 30.0f;
        case juce::AudioChannelSet::ChannelType::surround:
            return 180.0f;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::create5point0() == m_channelConfiguration
        || juce::AudioChannelSet::create5point1() == m_channelConfiguration
        || juce::AudioChannelSet::create5point1point2() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return -30.0f;
        case juce::AudioChannelSet::ChannelType::right:
            return 30.0f;
        case juce::AudioChannelSet::ChannelType::centre:
            return 0.0f;
        case juce::AudioChannelSet::ChannelType::LFE:
            return 0.0f;
        case juce::AudioChannelSet::ChannelType::leftSurround:
            return -110.0f;
        case juce::AudioChannelSet::ChannelType::rightSurround:
            return 110.0f;
        case juce::AudioChannelSet::ChannelType::topSideLeft:
            return -90.0f;
        case juce::AudioChannelSet::ChannelType::topSideRight:
            return 90.0f;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::create7point0() == m_channelConfiguration
        || juce::AudioChannelSet::create7point1() == m_channelConfiguration
        || juce::AudioChannelSet::create7point1point4() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return -30.0f;
        case juce::AudioChannelSet::ChannelType::right:
            return 30.0f;
        case juce::AudioChannelSet::ChannelType::centre:
            return 0.0f;
        case juce::AudioChannelSet::ChannelType::LFE:
            return 0.0f;
        case juce::AudioChannelSet::ChannelType::leftSurroundSide:
            return -100.0f;
        case juce::AudioChannelSet::ChannelType::rightSurroundSide:
            return 100.0f;
        case juce::AudioChannelSet::ChannelType::leftSurroundRear:
            return -145.0f;
        case juce::AudioChannelSet::ChannelType::rightSurroundRear:
            return 145.0f;
        case juce::AudioChannelSet::ChannelType::topFrontLeft:
            return -45.0f;
        case juce::AudioChannelSet::ChannelType::topFrontRight:
            return 45.0f;
        case juce::AudioChannelSet::ChannelType::topRearLeft:
            return -135.0f;
        case juce::AudioChannelSet::ChannelType::topRearRight:
            return 135.0f;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::create9point1point6() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return -30.0f;
        case juce::AudioChannelSet::ChannelType::right:
            return 30.0f;
        case juce::AudioChannelSet::ChannelType::centre:
            return 0.0f;
        case juce::AudioChannelSet::ChannelType::LFE:
            return 0.0f;
        case juce::AudioChannelSet::ChannelType::wideLeft:
            return -60.0f;
        case juce::AudioChannelSet::ChannelType::wideRight:
            return 60.0f;
        case juce::AudioChannelSet::ChannelType::leftSurroundSide:
            return -100.0f;
        case juce::AudioChannelSet::ChannelType::rightSurroundSide:
            return 100.0f;
        case juce::AudioChannelSet::ChannelType::leftSurroundRear:
            return -145.0f;
        case juce::AudioChannelSet::ChannelType::rightSurroundRear:
            return 145.0f;
        case juce::AudioChannelSet::ChannelType::topFrontLeft:
            return -45.0f;
        case juce::AudioChannelSet::ChannelType::topFrontRight:
            return 45.0f;
        case juce::AudioChannelSet::ChannelType::topSideLeft:
            return -90.0f;
        case juce::AudioChannelSet::ChannelType::topSideRight:
            return 90.0f;
        case juce::AudioChannelSet::ChannelType::topRearLeft:
            return -135.0f;
        case juce::AudioChannelSet::ChannelType::topRearRight:
            return 135.0f;
        default:
            jassertfalse;
        }
    }
    else
        jassertfalse;

    return 0.0f;
}

int TwoDFieldMultisliderComponent::getChannelNumberForChannelTypeInCurrentConfiguration(const juce::AudioChannelSet::ChannelType& channelType)
{
    if (juce::AudioChannelSet::mono() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::centre:
            return 1;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::stereo() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return 1;
        case juce::AudioChannelSet::ChannelType::right:
            return 2;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::createLCR() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return 1;
        case juce::AudioChannelSet::ChannelType::right:
            return 2;
        case juce::AudioChannelSet::ChannelType::centre:
            return 3;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::createLCRS() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return 1;
        case juce::AudioChannelSet::ChannelType::right:
            return 2;
        case juce::AudioChannelSet::ChannelType::centre:
            return 3;
        case juce::AudioChannelSet::ChannelType::surround:
            return 4;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::createLRS() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return 1;
        case juce::AudioChannelSet::ChannelType::right:
            return 2;
        case juce::AudioChannelSet::ChannelType::surround:
            return 3;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::create5point0() == m_channelConfiguration
        || juce::AudioChannelSet::create5point1() == m_channelConfiguration
        || juce::AudioChannelSet::create5point1point2() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return 1;
        case juce::AudioChannelSet::ChannelType::right:
            return 2;
        case juce::AudioChannelSet::ChannelType::centre:
            return 3;
        case juce::AudioChannelSet::ChannelType::LFE:
            return 4;
        case juce::AudioChannelSet::ChannelType::leftSurround:
            return 5;
        case juce::AudioChannelSet::ChannelType::rightSurround:
            return 6;
        case juce::AudioChannelSet::ChannelType::topSideLeft:
            return 7;
        case juce::AudioChannelSet::ChannelType::topSideRight:
            return 8;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::create7point0() == m_channelConfiguration
        || juce::AudioChannelSet::create7point1() == m_channelConfiguration
        || juce::AudioChannelSet::create7point1point4() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return 1;
        case juce::AudioChannelSet::ChannelType::right:
            return 2;
        case juce::AudioChannelSet::ChannelType::centre:
            return 3;
        case juce::AudioChannelSet::ChannelType::LFE:
            return 4;
        case juce::AudioChannelSet::ChannelType::leftSurroundSide:
            return 5;
        case juce::AudioChannelSet::ChannelType::rightSurroundSide:
            return 6;
        case juce::AudioChannelSet::ChannelType::leftSurroundRear:
            return 7;
        case juce::AudioChannelSet::ChannelType::rightSurroundRear:
            return 8;
        case juce::AudioChannelSet::ChannelType::topFrontLeft:
            return 9;
        case juce::AudioChannelSet::ChannelType::topFrontRight:
            return 10;
        case juce::AudioChannelSet::ChannelType::topRearLeft:
            return 11;
        case juce::AudioChannelSet::ChannelType::topRearRight:
            return 12;
        default:
            jassertfalse;
        }
    }
    else if (juce::AudioChannelSet::create9point1point6() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return 1;
        case juce::AudioChannelSet::ChannelType::right:
            return 2;
        case juce::AudioChannelSet::ChannelType::centre:
            return 3;
        case juce::AudioChannelSet::ChannelType::LFE:
            return 4;
        case juce::AudioChannelSet::ChannelType::wideLeft:
            return 5;
        case juce::AudioChannelSet::ChannelType::wideRight:
            return 6;
        case juce::AudioChannelSet::ChannelType::leftSurroundSide:
            return 7;
        case juce::AudioChannelSet::ChannelType::rightSurroundSide:
            return 8;
        case juce::AudioChannelSet::ChannelType::leftSurroundRear:
            return 9;
        case juce::AudioChannelSet::ChannelType::rightSurroundRear:
            return 10;
        case juce::AudioChannelSet::ChannelType::topFrontLeft:
            return 11;
        case juce::AudioChannelSet::ChannelType::topFrontRight:
            return 12;
        case juce::AudioChannelSet::ChannelType::topSideLeft:
            return 13;
        case juce::AudioChannelSet::ChannelType::topSideRight:
            return 14;
        case juce::AudioChannelSet::ChannelType::topRearLeft:
            return 15;
        case juce::AudioChannelSet::ChannelType::topRearRight:
            return 16;
        default:
            jassertfalse;
        }
    }
    else
        jassertfalse;

    return 1;
}

const juce::AudioChannelSet::ChannelType TwoDFieldMultisliderComponent::getChannelTypeForChannelNumberInCurrentConfiguration(int channelNumber)
{
    if (juce::AudioChannelSet::mono() == m_channelConfiguration)
    {
        switch (channelNumber)
        {
        case 1:
            return juce::AudioChannelSet::ChannelType::centre;
        default:
            break;
        }
    }
    else if (juce::AudioChannelSet::stereo() == m_channelConfiguration)
    {
        switch (channelNumber)
        {
        case 1: 
            return juce::AudioChannelSet::ChannelType::left;
        case 2: 
            return juce::AudioChannelSet::ChannelType::right;
        default:
            break;
        }
    }
    else if (juce::AudioChannelSet::createLCR() == m_channelConfiguration)
    {
        switch (channelNumber)
        {
        case 1:
            return juce::AudioChannelSet::ChannelType::left;
        case 2:
            return juce::AudioChannelSet::ChannelType::right;
        case 3:
            return juce::AudioChannelSet::ChannelType::centre;
        default:
            break;
        }
    }
    else if (juce::AudioChannelSet::createLCRS() == m_channelConfiguration)
    {
        switch (channelNumber)
        {
        case 1: 
            return juce::AudioChannelSet::ChannelType::left;
        case 2: 
            return juce::AudioChannelSet::ChannelType::right;
        case 3: 
            return juce::AudioChannelSet::ChannelType::centre;
        case 4: 
            return juce::AudioChannelSet::ChannelType::surround;
        default:
            break;
        }
    }
    else if (juce::AudioChannelSet::createLRS() == m_channelConfiguration)
    {
        switch (channelNumber)
        {
        case 1: 
            return juce::AudioChannelSet::ChannelType::left;
        case 2: 
            return juce::AudioChannelSet::ChannelType::right;
        case 3: 
            return juce::AudioChannelSet::ChannelType::surround;
        default:
            break;
        }
    }
    else if (juce::AudioChannelSet::create5point0() == m_channelConfiguration
        || juce::AudioChannelSet::create5point1() == m_channelConfiguration
        || juce::AudioChannelSet::create5point1point2() == m_channelConfiguration)
    {
        switch (channelNumber)
        {
        case 1: 
            return juce::AudioChannelSet::ChannelType::left;
        case 2: 
            return juce::AudioChannelSet::ChannelType::right;
        case 3: 
            return juce::AudioChannelSet::ChannelType::centre;
        case 4: 
            return juce::AudioChannelSet::ChannelType::LFE;
        case 5: 
            return juce::AudioChannelSet::ChannelType::leftSurround;
        case 6: 
            return juce::AudioChannelSet::ChannelType::rightSurround;
        case 7: 
            return juce::AudioChannelSet::ChannelType::topSideLeft;
        case 8: 
            return juce::AudioChannelSet::ChannelType::topSideRight;
        default:
            break;
        }
    }
    else if (juce::AudioChannelSet::create7point0() == m_channelConfiguration
        || juce::AudioChannelSet::create7point1() == m_channelConfiguration
        || juce::AudioChannelSet::create7point1point4() == m_channelConfiguration)
    {
        switch (channelNumber)
        {
        case 1: 
            return juce::AudioChannelSet::ChannelType::left;
        case 2: 
            return juce::AudioChannelSet::ChannelType::right;
        case 3: 
            return juce::AudioChannelSet::ChannelType::centre;
        case 4: 
            return juce::AudioChannelSet::ChannelType::LFE;
        case 5: 
            return juce::AudioChannelSet::ChannelType::leftSurroundSide;
        case 6: 
            return juce::AudioChannelSet::ChannelType::rightSurroundSide;
        case 7: 
            return juce::AudioChannelSet::ChannelType::leftSurroundRear;
        case 8: 
            return juce::AudioChannelSet::ChannelType::rightSurroundRear;
        case 9: 
            return juce::AudioChannelSet::ChannelType::topFrontLeft;
        case 10: 
            return juce::AudioChannelSet::ChannelType::topFrontRight;
        case 11: 
            return juce::AudioChannelSet::ChannelType::topRearLeft;
        case 12: 
            return juce::AudioChannelSet::ChannelType::topRearRight;
        default:
            break;
        }
    }
    else if (juce::AudioChannelSet::create9point1point6() == m_channelConfiguration)
    {
        switch (channelNumber)
        {
        case 1: 
            return juce::AudioChannelSet::ChannelType::left;
        case 2: 
            return juce::AudioChannelSet::ChannelType::right;
        case 3: 
            return juce::AudioChannelSet::ChannelType::centre;
        case 4: 
            return juce::AudioChannelSet::ChannelType::LFE;
        case 5: 
            return juce::AudioChannelSet::ChannelType::wideLeft;
        case 6: 
            return juce::AudioChannelSet::ChannelType::wideRight;
        case 7: 
            return juce::AudioChannelSet::ChannelType::leftSurroundSide;
        case 8: 
            return juce::AudioChannelSet::ChannelType::rightSurroundSide;
        case 9: 
            return juce::AudioChannelSet::ChannelType::leftSurroundRear;
        case 10: 
            return juce::AudioChannelSet::ChannelType::rightSurroundRear;
        case 11: 
            return juce::AudioChannelSet::ChannelType::topFrontLeft;
        case 12: 
            return juce::AudioChannelSet::ChannelType::topFrontRight;
        case 13: 
            return juce::AudioChannelSet::ChannelType::topSideLeft;
        case 14: 
            return juce::AudioChannelSet::ChannelType::topSideRight;
        case 15: 
            return juce::AudioChannelSet::ChannelType::topRearLeft;
        case 16: 
            return juce::AudioChannelSet::ChannelType::topRearRight;
        default:
            break;
        }
    }
    else
        jassertfalse;

    return juce::AudioChannelSet::ChannelType::unknown;
}

void TwoDFieldMultisliderComponent::setClockwiseOrderedChannelTypesForCurrentConfiguration()
{
    m_clockwiseOrderedChannelTypes.clear();
    m_clockwiseOrderedHeightChannelTypes.clear();
    m_directionLessChannelTypes.clear();

    if (juce::AudioChannelSet::mono() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = m_channelConfiguration.getChannelTypes();
    }
    else if (juce::AudioChannelSet::stereo() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = { 
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::right };
    }
    else if (juce::AudioChannelSet::createLCR() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = { 
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::centre,
            juce::AudioChannelSet::ChannelType::right };
    }
    else if (juce::AudioChannelSet::createLCRS() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = { 
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::centre,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::surround };
    }
    else if (juce::AudioChannelSet::createLRS() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = { 
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::surround };
    }
    else if (juce::AudioChannelSet::create5point0() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = { 
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::centre,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::rightSurround,
            juce::AudioChannelSet::ChannelType::leftSurround,
        };
    }
    else if (juce::AudioChannelSet::create5point1() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = {
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::centre,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::rightSurround,
            juce::AudioChannelSet::ChannelType::leftSurround
        };
        m_directionLessChannelTypes = {
            juce::AudioChannelSet::ChannelType::LFE
        };
    }
    else if (juce::AudioChannelSet::create5point1point2() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = {
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::centre,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::rightSurround,
            juce::AudioChannelSet::ChannelType::leftSurround
        };
        m_clockwiseOrderedHeightChannelTypes = {
            juce::AudioChannelSet::ChannelType::topSideLeft,
            juce::AudioChannelSet::ChannelType::topSideRight
        };
        m_directionLessChannelTypes = {
            juce::AudioChannelSet::ChannelType::LFE
        };
    }
    else if (juce::AudioChannelSet::create7point0() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = {
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::centre,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::rightSurroundSide,
            juce::AudioChannelSet::ChannelType::rightSurroundRear,
            juce::AudioChannelSet::ChannelType::leftSurroundRear,
            juce::AudioChannelSet::ChannelType::leftSurroundSide
        };
    }
    else if (juce::AudioChannelSet::create7point1() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = { 
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::centre,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::rightSurroundSide,
            juce::AudioChannelSet::ChannelType::rightSurroundRear,
            juce::AudioChannelSet::ChannelType::leftSurroundRear,
            juce::AudioChannelSet::ChannelType::leftSurroundSide
        };
        m_directionLessChannelTypes = {
            juce::AudioChannelSet::ChannelType::LFE
        };
    }
    else if (juce::AudioChannelSet::create7point1point4() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = {
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::centre,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::rightSurroundSide,
            juce::AudioChannelSet::ChannelType::rightSurroundRear,
            juce::AudioChannelSet::ChannelType::leftSurroundRear,
            juce::AudioChannelSet::ChannelType::leftSurroundSide
        };
        m_clockwiseOrderedHeightChannelTypes = {
            juce::AudioChannelSet::ChannelType::topFrontLeft,
            juce::AudioChannelSet::ChannelType::topFrontRight,
            juce::AudioChannelSet::ChannelType::topRearRight,
            juce::AudioChannelSet::ChannelType::topRearLeft
        };
        m_directionLessChannelTypes = {
            juce::AudioChannelSet::ChannelType::LFE
        };
    }
    else if (juce::AudioChannelSet::create9point1point6() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = {
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::centre,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::wideRight,
            juce::AudioChannelSet::ChannelType::rightSurroundSide,
            juce::AudioChannelSet::ChannelType::rightSurroundRear,
            juce::AudioChannelSet::ChannelType::leftSurroundRear,
            juce::AudioChannelSet::ChannelType::leftSurroundSide,
            juce::AudioChannelSet::ChannelType::wideLeft
        };
        m_clockwiseOrderedHeightChannelTypes = {
            juce::AudioChannelSet::ChannelType::topFrontLeft,
            juce::AudioChannelSet::ChannelType::topFrontRight,
            juce::AudioChannelSet::ChannelType::topSideRight,
            juce::AudioChannelSet::ChannelType::topRearRight,
            juce::AudioChannelSet::ChannelType::topRearLeft,
            juce::AudioChannelSet::ChannelType::topSideLeft
        };
        m_directionLessChannelTypes = {
            juce::AudioChannelSet::ChannelType::LFE
        };
    }
    else
        jassertfalse;
}

void TwoDFieldMultisliderComponent::rebuildDirectionslessChannelSliders()
{
    m_directionslessChannelSliders.clear();
    for (auto const& channelType : m_directionLessChannelTypes)
    {
        m_directionslessChannelSliders[channelType] = std::make_unique<JUCEAppBasics::ToggleStateSlider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
        m_directionslessChannelSliders[channelType]->setColour(juce::Slider::ColourIds::trackColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
        m_directionslessChannelSliders[channelType]->setRange(0.0, 1.0, 0.01);
        m_directionslessChannelSliders[channelType]->setToggleState(false, juce::dontSendNotification);
        m_directionslessChannelSliders[channelType]->displayValueConverter = [](double val) { return juce::String(juce::Decibels::gainToDecibels(val, static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB())), 1) + " dB"; };
        m_directionslessChannelSliders[channelType]->onToggleStateChange = [this, channelType]() {
            std::map<std::uint16_t, std::map<std::uint16_t, bool >> states;
            if (0 != m_currentlySelectedInput)
                states[m_currentlySelectedInput][std::uint16_t(getChannelNumberForChannelTypeInCurrentConfiguration(channelType))] = m_directionslessChannelSliders[channelType]->getToggleState();
            else
            {
                for (auto& ioValKV : m_inputToOutputVals)
                {
                    ioValKV.second[channelType].first = m_directionslessChannelSliders[channelType]->getToggleState();
                    states[ioValKV.first][std::uint16_t(getChannelNumberForChannelTypeInCurrentConfiguration(channelType))] = m_directionslessChannelSliders[channelType]->getToggleState();
                }
            }

            if (onInputToOutputStatesChanged)
                onInputToOutputStatesChanged(states);
        };
        m_directionslessChannelSliders[channelType]->onValueChange = [this, channelType]() {
            std::map<std::uint16_t, std::map<std::uint16_t, float >> values;
            if (0 != m_currentlySelectedInput)
                values[m_currentlySelectedInput][std::uint16_t(getChannelNumberForChannelTypeInCurrentConfiguration(channelType))] = float(m_directionslessChannelSliders[channelType]->getValue());
            else
            {
                auto latestValue = m_directionslessChannelSliders[channelType]->getValue();
                auto latestDelta = latestValue - m_directionlessSliderRelRef[channelType];
                for (auto& ioValKV : m_inputToOutputVals)
                {
                    ioValKV.second[channelType].second += float(latestDelta);
                    values[ioValKV.first][std::uint16_t(getChannelNumberForChannelTypeInCurrentConfiguration(channelType))] = ioValKV.second[channelType].second;
                    //DBG(juce::String(ioValKV.first) << ">" << juce::AudioChannelSet::getAbbreviatedChannelTypeName(channelType) << " val:" << ioValKV.second[channelType].second);
                }
                m_directionlessSliderRelRef[channelType] = latestValue;
            }

            if (onInputToOutputValuesChanged)
                onInputToOutputValuesChanged(values);
        };
        addAndMakeVisible(m_directionslessChannelSliders[channelType].get());

        m_directionslessChannelLabels[channelType] = std::make_unique<juce::Label>();
        m_directionslessChannelLabels[channelType]->setText(juce::AudioChannelSet::getAbbreviatedChannelTypeName(channelType), juce::dontSendNotification);
        m_directionslessChannelLabels[channelType]->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(m_directionslessChannelLabels[channelType].get());
    }
}

float TwoDFieldMultisliderComponent::getRequiredAspectRatio()
{
    auto coreTwoDFieldOnly = usesPositionedChannels() && !usesPositionedHeightChannels() && !usesDirectionlessChannels();
    auto coreTwoDFieldWithMeterbridge = usesPositionedChannels() && !usesPositionedHeightChannels() && usesDirectionlessChannels();
    auto bothTwoDFields = usesPositionedChannels() && usesPositionedHeightChannels() && !usesDirectionlessChannels();
    auto bothTwoDFieldsWithMeterbridge = usesPositionedChannels() && usesPositionedHeightChannels() && usesDirectionlessChannels();
    if (coreTwoDFieldOnly)
        return 1.0f;
    else if (coreTwoDFieldWithMeterbridge)
        return (10.0f / 11.0f);
    else if (bothTwoDFields)
        return (10.0f / 12.0f);
    else if (bothTwoDFieldsWithMeterbridge)
        return (10.0f / 13.0f);
    
    jassertfalse;
    return 0.0f;
}

const juce::Array<juce::AudioChannelSet::ChannelType>& TwoDFieldMultisliderComponent::getOutputsInLayer(const ChannelLayer& layer)
{
    if (ChannelLayer::Positioned == layer)
        return m_clockwiseOrderedChannelTypes;
    else if (ChannelLayer::PositionedHeight == layer)
        return m_clockwiseOrderedHeightChannelTypes;
    else
        return m_directionLessChannelTypes;
}

const juce::Array<juce::AudioChannelSet::ChannelType> TwoDFieldMultisliderComponent::getDirectiveOutputsNotInLayer(const ChannelLayer& layer)
{
    if (ChannelLayer::Positioned == layer)
        return m_clockwiseOrderedHeightChannelTypes;
    else if (ChannelLayer::PositionedHeight == layer)
        return m_clockwiseOrderedChannelTypes;
    else
    {
        auto types = m_clockwiseOrderedChannelTypes;
        types.addArray(m_clockwiseOrderedHeightChannelTypes);
        return types;
    }
}


}

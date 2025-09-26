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

#include "TwoDFieldOutputComponent.h"

#include <CustomLookAndFeel.h>


namespace Mema
{

//#define PAINTINGHELPER

//==============================================================================
TwoDFieldOutputComponent::TwoDFieldOutputComponent()
    : TwoDFieldBase(), AbstractAudioVisualizer()
{
    setUsesValuesInDB(true);
}

TwoDFieldOutputComponent::~TwoDFieldOutputComponent()
{
}

void TwoDFieldOutputComponent::paint (juce::Graphics& g)
{
    AbstractAudioVisualizer::paint(g);

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // paint the level indications where applicable
    if (!m_positionedChannelsArea.isEmpty())
        paintCircularLevelIndication(g, m_positionedChannelsArea, m_channelLevelMaxPoints, m_clockwiseOrderedChannelTypes);
    if (!m_positionedHeightChannelsArea.isEmpty())
        paintCircularLevelIndication(g, m_positionedHeightChannelsArea, m_channelHeightLevelMaxPoints, m_clockwiseOrderedHeightChannelTypes);
    if (!m_directionlessChannelsArea.isEmpty())
        paintLevelMeterIndication(g, m_directionlessChannelsArea, m_directionLessChannelTypes);

    // draw dBFS
    g.setFont(12.0f);
    g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOffId));
    juce::String rangeText;
    if (getUsesValuesInDB())
        rangeText = juce::String(ProcessorDataAnalyzer::getGlobalMindB()) + " ... " + juce::String(ProcessorDataAnalyzer::getGlobalMaxdB()) + " dBFS";
    else
        rangeText = "0 ... 1";
    g.drawText(rangeText, getLocalBounds(), juce::Justification::topRight, true);
}

void TwoDFieldOutputComponent::paintCircularLevelIndication(juce::Graphics& g, const juce::Rectangle<float>& circleArea, const std::map<int, juce::Point<float>>& channelLevelMaxPoints, const juce::Array<juce::AudioChannelSet::ChannelType>& channelsToPaint)
{
#if defined DEBUG && defined PAINTINGHELPER
    g.setColour(juce::Colours::blueviolet);
    g.drawRect(circleArea);
#endif

    // fill circle background
    g.setColour(getLookAndFeel().findColour(juce::Slider::backgroundColourId));
    g.fillEllipse(circleArea);

#if defined DEBUG && defined PAINTINGHELPER
    g.setColour(juce::Colours::red);
    g.drawRect(circleArea);
    g.setColour(juce::Colours::blue);
    g.drawRect(getLocalBounds());
#endif


    const float meterWidth = 5.0f;
    const float halfMeterWidth = 2.0f;


    // helper std::function to avoid codeclones below
    auto calcLevelVals = [=](std::map<int, float>& levels, bool isHold, bool isPeak, bool isRms) {
        for (auto const& channelType : channelsToPaint)
        {
            if (isHold)
            {
                if (getUsesValuesInDB())
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).GetFactorHOLDdB();
                else
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).hold;
            }
            else if (isPeak)
            {
                if (getUsesValuesInDB())
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).GetFactorPEAKdB();
                else
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).peak;
            }
            else if (isRms)
            {
                if (getUsesValuesInDB())
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).GetFactorRMSdB();
                else
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).rms;
            }
        }
    };
    // calculate hold values
    std::map<int, float> holdLevels;
    calcLevelVals(holdLevels, true, false, false);
    // calculate peak values
    std::map<int, float> peakLevels;
    calcLevelVals(peakLevels, false, true, false);
    // calculate rms values    
    std::map<int, float> rmsLevels;
    calcLevelVals(rmsLevels, false, false, true);


    auto circleCenter = circleArea.getCentre();

    // prepare max points
    std::map<int, juce::Point<float>> centerToMaxVectors;
    std::map<int, juce::Point<float>> meterWidthOffsetVectors;
    for (int i = 0; i < channelsToPaint.size(); i++)
    {
        auto const& channelType = channelsToPaint[i];
        if (0 < channelLevelMaxPoints.count(channelType))
        {
            auto angleRad = juce::degreesToRadians(getAngleForChannelTypeInCurrentConfiguration(channelType));
            centerToMaxVectors[channelType] = circleCenter - channelLevelMaxPoints.at(channelType);
            meterWidthOffsetVectors[channelType] = { cosf(angleRad) * halfMeterWidth, sinf(angleRad) * halfMeterWidth };
        }
    }

    // helper std::function to avoid codeclones below
    auto createAndPaintLevelPath = [=](std::map<int, juce::Point<float>>& centerToMaxPoints, std::map<int, juce::Point<float>>& meterWidthOffsetPoints, std::map<int, float>& levels, juce::Graphics& g, const juce::Colour& colour, bool stroke) {
        juce::Path path;
        auto pathStarted = false;
        for (auto const& channelType : channelsToPaint)
        {
            auto channelMaxPoint = circleCenter - (centerToMaxPoints[channelType] * levels[channelType]);

            if (!pathStarted)
            {
                path.startNewSubPath(channelMaxPoint - meterWidthOffsetPoints[channelType]);
                pathStarted = true;
            }
            else
                path.lineTo(channelMaxPoint - meterWidthOffsetPoints[channelType]);

            path.lineTo(channelMaxPoint + meterWidthOffsetPoints[channelType]);
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
    // paint hold values as path
    createAndPaintLevelPath(centerToMaxVectors, meterWidthOffsetVectors, holdLevels, g, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringHoldColourId), true);
    // paint peak values as path
    createAndPaintLevelPath(centerToMaxVectors, meterWidthOffsetVectors, peakLevels, g, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringPeakColourId), false);
    // paint rms values as path
    createAndPaintLevelPath(centerToMaxVectors, meterWidthOffsetVectors, rmsLevels, g, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId), false);


    // helper std::function to avoid codeclones below
    auto paintLevelMeterLines = [=](std::map<int, juce::Point<float>>& centerToMaxPoints, std::map<int, juce::Point<float>>& meterWidthOffsetPoints, std::map<int, float>& levels, juce::Graphics& g, const juce::Colour& colour, bool isHoldLine) {
        g.setColour(colour);
        for (auto const& channelType : channelsToPaint)
        {
            auto channelMaxPoint = circleCenter - (centerToMaxPoints[channelType] * levels[channelType]);

            if (isHoldLine)
                g.drawLine(juce::Line<float>(channelMaxPoint - meterWidthOffsetPoints[channelType], channelMaxPoint + meterWidthOffsetPoints[channelType]), 1.0f);
            else
                g.drawLine(juce::Line<float>(circleCenter, channelMaxPoint), meterWidth);
        }
    };
    // paint hold values as max line
    paintLevelMeterLines(centerToMaxVectors, meterWidthOffsetVectors, holdLevels, g, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringHoldColourId), true);
    // paint peak values as line
    paintLevelMeterLines(centerToMaxVectors, meterWidthOffsetVectors, peakLevels, g, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringPeakColourId), false);
    // paint rms values as line
    paintLevelMeterLines(centerToMaxVectors, meterWidthOffsetVectors, rmsLevels, g, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId), false);

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

void TwoDFieldOutputComponent::paintLevelMeterIndication(juce::Graphics& g, const juce::Rectangle<float>& levelMeterArea, const juce::Array<juce::AudioChannelSet::ChannelType>& channelsToPaint)
{
#if defined DEBUG && defined PAINTINGHELPER
    g.setColour(juce::Colours::aqua);
    g.drawRect(levelMeterArea);
#endif

    auto channelCount = channelsToPaint.size();
    auto margin = levelMeterArea.getWidth() / ((2 * channelCount) + 1);

    auto visuArea = levelMeterArea;
    auto visuAreaOrigY = visuArea.getBottom();

    // draw meters
    auto meterSpacing = margin;
    auto meterThickness = float(visuArea.getWidth() - (channelCount)*meterSpacing) / float(channelCount);
    auto meterMaxLength = visuArea.getHeight();
    auto meterLeft = levelMeterArea.getX() + 0.5f * meterSpacing;

    g.setFont(14.0f);
    for (auto const& channelType : channelsToPaint)
    {
        auto level = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType));
        float peakMeterLength{ 0 };
        float rmsMeterLength{ 0 };
        float holdMeterLength{ 0 };
        if (getUsesValuesInDB())
        {
            peakMeterLength = meterMaxLength * level.GetFactorPEAKdB();
            rmsMeterLength = meterMaxLength * level.GetFactorRMSdB();
            holdMeterLength = meterMaxLength * level.GetFactorHOLDdB();
        }
        else
        {
            peakMeterLength = meterMaxLength * level.peak;
            rmsMeterLength = meterMaxLength * level.rms;
            holdMeterLength = meterMaxLength * level.hold;
        }

        // peak bar
        g.setColour(getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringPeakColourId));
        g.fillRect(juce::Rectangle<float>(meterLeft, visuAreaOrigY - peakMeterLength, meterThickness, peakMeterLength));
        // rms bar
        g.setColour(getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
        g.fillRect(juce::Rectangle<float>(meterLeft, visuAreaOrigY - rmsMeterLength, meterThickness, rmsMeterLength));
        // hold strip
        g.setColour(getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringHoldColourId));
        g.drawLine(juce::Line<float>(meterLeft, visuAreaOrigY - holdMeterLength, meterLeft + meterThickness, visuAreaOrigY - holdMeterLength));
        // channel # label
        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOffId));
        g.drawText(juce::AudioChannelSet::getAbbreviatedChannelTypeName(channelType), juce::Rectangle<float>(meterLeft - (0.5f * meterSpacing), visuAreaOrigY - float(margin + 2), meterThickness + meterSpacing, float(margin)), juce::Justification::centred);

        meterLeft += meterThickness + meterSpacing;
    }

    // draw a simple baseline
    g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOffId));
    g.drawLine(juce::Line<float>(levelMeterArea.getX(), visuAreaOrigY, levelMeterArea.getX() + visuArea.getWidth(), visuAreaOrigY));
}

void TwoDFieldOutputComponent::resized()
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

    AbstractAudioVisualizer::resized();
}

void TwoDFieldOutputComponent::processingDataChanged(AbstractProcessorData *data)
{
    if(!data)
        return;
    
    switch(data->GetDataType())
    {
        case AbstractProcessorData::Level:
            m_levelData = *(static_cast<ProcessorLevelData*>(data));
            notifyChanges();
            break;
        case AbstractProcessorData::AudioSignal:
        case AbstractProcessorData::Spectrum:
        case AbstractProcessorData::Invalid:
        default:
            break;
    }
}


}

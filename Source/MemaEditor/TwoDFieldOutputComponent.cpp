/* Copyright (c) 2024, Christian Ahrens
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


namespace Mema
{

//#define PAINTINGHELPER

//==============================================================================
TwoDFieldOutputComponent::TwoDFieldOutputComponent()
    :   AbstractAudioVisualizer()
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
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker());
    g.fillEllipse(circleArea);

#if defined DEBUG && defined PAINTINGHELPER
    g.setColour(juce::Colours::red);
    g.drawRect(circleArea);
    g.setColour(juce::Colours::blue);
    g.drawRect(getLocalBounds());
#endif


    auto circleCenter = circleArea.getCentre();

    // prepare max points
    std::map<int, juce::Point<float>> maxPoints;
    for (auto const& channelType : channelsToPaint)
        if (0 < channelLevelMaxPoints.count(channelType))
            maxPoints[channelType] = circleCenter - channelLevelMaxPoints.at(channelType);

    // helper std::function to avoid codeclones below
    auto calcLevelVals = [=](std::map<int, float>& levels, bool hold, bool peak, bool rms) {
        for (auto const& channelType : channelsToPaint)
        {
            if (hold)
            {
                if (getUsesValuesInDB())
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).GetFactorHOLDdB();
                else
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).hold;
            }
            else if (peak)
            {
                if (getUsesValuesInDB())
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).GetFactorPEAKdB();
                else
                    levels[channelType] = m_levelData.GetLevel(getChannelNumberForChannelTypeInCurrentConfiguration(channelType)).peak;
            }
            else if (rms)
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

    // helper std::function to avoid codeclones below
    auto createAndPaintLevelPath = [=](std::map<int, juce::Point<float>> points, std::map<int, float>& levels, juce::Graphics& g, const juce::Colour& colour, bool stroke) {
        juce::Path path;
        auto pathStarted = false;
        for (auto const& channelType : channelsToPaint)
        {
            auto channelMaxPoint = points[channelType] * levels[channelType];

            if (!pathStarted)
            {
                path.startNewSubPath(circleCenter - juce::Point<float>(channelMaxPoint.getX(), channelMaxPoint.getY()));
                pathStarted = true;
            }
            else
                path.lineTo(circleCenter - juce::Point<float>(channelMaxPoint.getX(), channelMaxPoint.getY()));
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
    createAndPaintLevelPath(maxPoints, holdLevels, g, juce::Colours::grey, true);

    // paint peak values as path
    createAndPaintLevelPath(maxPoints, peakLevels, g, juce::Colours::forestgreen.darker(), false);

    // paint rms values as path
    createAndPaintLevelPath(maxPoints, rmsLevels, g, juce::Colours::forestgreen, false);

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
        g.setColour(juce::Colours::forestgreen.darker());
        g.fillRect(juce::Rectangle<float>(meterLeft, visuAreaOrigY - peakMeterLength, meterThickness, peakMeterLength));
        // rms bar
        g.setColour(juce::Colours::forestgreen);
        g.fillRect(juce::Rectangle<float>(meterLeft, visuAreaOrigY - rmsMeterLength, meterThickness, rmsMeterLength));
        // hold strip
        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOffId));
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
        auto xLength = sinf(juce::MathConstants<float>::pi / 180.0f * (getAngleForChannelTypeInCurrentConfiguration(channelType))) * (m_positionedChannelsArea.getHeight() / 2);
        auto yLength = cosf(juce::MathConstants<float>::pi / 180.0f * (getAngleForChannelTypeInCurrentConfiguration(channelType))) * (m_positionedChannelsArea.getWidth() / 2);
        m_channelLevelMaxPoints[channelType] = m_positionedChannelsArea.getCentre() + juce::Point<float>(xLength, -yLength);
    }

    for (auto const& channelType : m_clockwiseOrderedHeightChannelTypes)
    {
        auto xLength = sinf(juce::MathConstants<float>::pi / 180.0f * (getAngleForChannelTypeInCurrentConfiguration(channelType))) * (m_positionedHeightChannelsArea.getHeight() / 2);
        auto yLength = cosf(juce::MathConstants<float>::pi / 180.0f * (getAngleForChannelTypeInCurrentConfiguration(channelType))) * (m_positionedHeightChannelsArea.getWidth() / 2);
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

bool TwoDFieldOutputComponent::setChannelConfiguration(const juce::AudioChannelSet& channelLayout)
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
        setClockwiseOrderedChannelTypesForCurrentConfiguration();

    return wasUpdated;
}

const juce::Array<juce::AudioChannelSet>& TwoDFieldOutputComponent::getSupportedChannelConfigurations()
{
    return m_supportedChannelConfigurations;
}

float TwoDFieldOutputComponent::getAngleForChannelTypeInCurrentConfiguration(const juce::AudioChannelSet::ChannelType& channelType)
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
    else
        jassertfalse;

    return 0.0f;
}

int TwoDFieldOutputComponent::getChannelNumberForChannelTypeInCurrentConfiguration(const juce::AudioChannelSet::ChannelType& channelType)
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
    else
        jassertfalse;

    return 1;
}

void TwoDFieldOutputComponent::setClockwiseOrderedChannelTypesForCurrentConfiguration()
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
    else
        jassertfalse;
}

float TwoDFieldOutputComponent::getRequiredAspectRatio()
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


}

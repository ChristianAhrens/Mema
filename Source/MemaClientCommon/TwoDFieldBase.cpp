/* Copyright (c) 2025, Christian Ahrens
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

#include "TwoDFieldBase.h"


namespace Mema
{


//==============================================================================
TwoDFieldBase::TwoDFieldBase()
{
}

TwoDFieldBase::~TwoDFieldBase()
{
}

bool TwoDFieldBase::setChannelConfiguration(const juce::AudioChannelSet& channelLayout)
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
    }

    return wasUpdated;
}

const juce::Array<juce::AudioChannelSet>& TwoDFieldBase::getSupportedChannelConfigurations()
{
    return m_supportedChannelConfigurations;
}

float TwoDFieldBase::getAngleForChannelTypeInCurrentConfiguration(const juce::AudioChannelSet::ChannelType& channelType)
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
    else if (juce::AudioChannelSet::quadraphonic() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return -45.0f;
        case juce::AudioChannelSet::ChannelType::right:
            return 45.0f;
        case juce::AudioChannelSet::ChannelType::leftSurround:
            return -135.0f;
        case juce::AudioChannelSet::ChannelType::rightSurround:
            return 135.0f;
        default:
            jassertfalse;
        }
    }
    else
        jassertfalse;

    return 0.0f;
}

int TwoDFieldBase::getChannelNumberForChannelTypeInCurrentConfiguration(const juce::AudioChannelSet::ChannelType& channelType)
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
    else if (juce::AudioChannelSet::quadraphonic() == m_channelConfiguration)
    {
        switch (channelType)
        {
        case juce::AudioChannelSet::ChannelType::left:
            return 1;
        case juce::AudioChannelSet::ChannelType::right:
            return 2;
        case juce::AudioChannelSet::ChannelType::leftSurround:
            return 3;
        case juce::AudioChannelSet::ChannelType::rightSurround:
            return 4;
        default:
            jassertfalse;
        }
    }
    else
        jassertfalse;

    return 1;
}

const juce::AudioChannelSet::ChannelType TwoDFieldBase::getChannelTypeForChannelNumberInCurrentConfiguration(int channelNumber)
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
    else if (juce::AudioChannelSet::quadraphonic() == m_channelConfiguration)
    {
        switch (channelNumber)
        {
        case 1:
            return juce::AudioChannelSet::ChannelType::left;
        case 2:
            return juce::AudioChannelSet::ChannelType::right;
        case 3:
            return juce::AudioChannelSet::ChannelType::leftSurround;
        case 4:
            return juce::AudioChannelSet::ChannelType::rightSurround;
        default:
            break;
        }
    }
    else
        jassertfalse;

    return juce::AudioChannelSet::ChannelType::unknown;
}

void TwoDFieldBase::setClockwiseOrderedChannelTypesForCurrentConfiguration()
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
    else if (juce::AudioChannelSet::quadraphonic() == m_channelConfiguration)
    {
        m_clockwiseOrderedChannelTypes = {
            juce::AudioChannelSet::ChannelType::left,
            juce::AudioChannelSet::ChannelType::right,
            juce::AudioChannelSet::ChannelType::rightSurround,
            juce::AudioChannelSet::ChannelType::leftSurround,
        };
    }
    else
        jassertfalse;
}

float TwoDFieldBase::getRequiredAspectRatio()
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
    
    return 0.0f;
}

const juce::Array<juce::AudioChannelSet::ChannelType>& TwoDFieldBase::getOutputsInLayer(const ChannelLayer& layer)
{
    if (ChannelLayer::Positioned == layer)
        return m_clockwiseOrderedChannelTypes;
    else if (ChannelLayer::PositionedHeight == layer)
        return m_clockwiseOrderedHeightChannelTypes;
    else
        return m_directionLessChannelTypes;
}

const juce::Array<juce::AudioChannelSet::ChannelType> TwoDFieldBase::getDirectiveOutputsNotInLayer(const ChannelLayer& layer)
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

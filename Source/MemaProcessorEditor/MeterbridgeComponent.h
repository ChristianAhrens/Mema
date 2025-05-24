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

#include "AbstractAudioVisualizer.h"


namespace Mema
{

//==============================================================================
/*
*/
class MeterbridgeComponent : public AbstractAudioVisualizer
{
public:
    enum Direction
    {
        Horizontal,
        Vertical
    };

public:
    MeterbridgeComponent();
    MeterbridgeComponent(Direction direction);
    ~MeterbridgeComponent();

    //==============================================================================
    void paint (Graphics&) override;

    //==============================================================================
    void processingDataChanged(AbstractProcessorData* data) override;

    //==============================================================================
    void setDirection(Direction direction);
    void setChannelCount(int channelCount);
    int getChannelCount();

private:
    ProcessorLevelData  m_levelData;
    Direction m_direction{ Vertical };
    int m_channelCount{ 0 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MeterbridgeComponent)
};

}
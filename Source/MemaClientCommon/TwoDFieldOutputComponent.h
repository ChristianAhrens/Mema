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
#include "TwoDFieldBase.h"

namespace Mema
{

//==============================================================================
/*
*/
class TwoDFieldOutputComponent  :   public TwoDFieldBase, public AbstractAudioVisualizer
{
public:
    TwoDFieldOutputComponent();
    ~TwoDFieldOutputComponent();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    //==============================================================================
    void processingDataChanged(AbstractProcessorData *data) override;

private:
    //==============================================================================
    void paintCircularLevelIndication(juce::Graphics& g, const juce::Rectangle<float>& circleArea, const std::map<int, juce::Point<float>>& channelLevelMaxPoints, const juce::Array<juce::AudioChannelSet::ChannelType>& channelsToPaint);
    void paintLevelMeterIndication(juce::Graphics& g, const juce::Rectangle<float>& levelMeterArea, const juce::Array<juce::AudioChannelSet::ChannelType>& channelsToPaint);

    ProcessorLevelData  m_levelData;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TwoDFieldOutputComponent)
};

}
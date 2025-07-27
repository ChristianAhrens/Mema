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

#pragma once

#include <JuceHeader.h>

#include "../MemaProcessorEditor/AbstractAudioVisualizer.h"

namespace Mema
{

//==============================================================================
/*
*/
class WaveformAudioComponent    :   public AbstractAudioVisualizer
{
public:
    //==============================================================================
    WaveformAudioComponent();
    ~WaveformAudioComponent();

    //==============================================================================
    void setNumVisibleChannels(int numChannels);
    
    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;
    
    //==============================================================================
    void processingDataChanged(AbstractProcessorData *data) override;

private:
    //==============================================================================
    std::unique_ptr<juce::AudioVisualiserComponent> m_waveformsComponent;
    std::unique_ptr<juce::DrawableButton>           m_chNumSelButton;

    //==============================================================================
    int m_numAvailableChannels = 0;
    int m_numVisibleChannels = 1;
    int m_legendWidth = 20;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformAudioComponent)
};


}
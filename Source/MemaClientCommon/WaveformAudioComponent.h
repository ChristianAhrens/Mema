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
    WaveformAudioComponent();
    ~WaveformAudioComponent();
    
    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    //==============================================================================
    void processingDataChanged(AbstractProcessorData *data) override;

private:
    std::unique_ptr<juce::AudioThumbnail>       m_thumbnail;
    std::unique_ptr<juce::AudioThumbnailCache>  m_thumbnailCache;

    juce::AudioBuffer<float>    m_buffer;
    int                         m_bufferPos;
    int                         m_bufferTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveformAudioComponent)
};


}
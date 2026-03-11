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

#include "AbstractProcessorData.h"

namespace Mema
{

/** @class ProcessorAudioSignalData @brief Audio buffer data object carrying raw PCM samples plus sample-rate metadata. */
class ProcessorAudioSignalData :    public AbstractProcessorData,
public juce::AudioBuffer<float>
{
public:
    ProcessorAudioSignalData();
    ~ProcessorAudioSignalData();

    /** @brief Sets the number of channels in the audio buffer. */
    void SetChannelCount(unsigned long count) override;
    /** @brief Returns the number of channels in the audio buffer. */
    unsigned long GetChannelCount() override;

    /** @brief Sets the sample rate associated with this buffer. */
    void SetSampleRate(unsigned long rate);
    /** @brief Returns the sample rate associated with this buffer. */
    unsigned long GetSampleRate();

private:
    unsigned long m_sampleRate; ///< Sample rate of the audio buffer in Hz.
    
};

}

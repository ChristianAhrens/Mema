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

#include "AbstractProcessorData.h"

namespace Mema
{

/** @class ProcessorLevelData @brief Level-metering data object carrying peak, RMS, and hold values for each channel. */
class ProcessorLevelData : public AbstractProcessorData
{
public:
    /** @brief Per-channel level values in both linear and dB domains. */
    struct LevelVal
    {
        LevelVal()
        {
            peak = 0.0f;
            rms = 0.0f;
            hold = 0.0f;
            peakdB = 0.0f;
            rmsdB = 0.0f;
            holddB = 0.0f;
            minusInfdb = -10000.0f;
        }
        LevelVal(float p, float r, float h, float infdb = -100.0f)
        {
            peak = p;
            rms = r;
            hold = h;
            peakdB = juce::Decibels::gainToDecibels(peak, infdb);
            rmsdB = juce::Decibels::gainToDecibels(rms, infdb);
            holddB = juce::Decibels::gainToDecibels(hold, infdb);
            minusInfdb = infdb;
        }

        float GetFactorRMSdB()
        {
            return (-1 * minusInfdb + rmsdB) / (-1 * minusInfdb);
        }
        float GetFactorPEAKdB()
        {
            return (-1 * minusInfdb + peakdB) / (-1 * minusInfdb);
        }
        float GetFactorHOLDdB()
        {
            return (-1 * minusInfdb + holddB) / (-1 * minusInfdb);
        }
        
        float   peak;       ///< Linear peak level.
        float   rms;        ///< Linear RMS level.
        float   hold;       ///< Linear hold level.
        float   peakdB;     ///< Peak level in dB.
        float   rmsdB;      ///< RMS level in dB.
        float   holddB;     ///< Hold level in dB.
        float   minusInfdb; ///< Value used as -infinity in dB calculations.
    };
    
public:
    ProcessorLevelData();
    ~ProcessorLevelData();
    
    void SetLevel(unsigned long channel, LevelVal level);
    LevelVal GetLevel(unsigned long channel);
    
    void SetChannelCount(unsigned long count) override;
    unsigned long GetChannelCount() override;
    
private:
    std::map<unsigned long, LevelVal>    m_levelMap;
};

}

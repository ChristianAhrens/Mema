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

class ProcessorSpectrumData : public AbstractProcessorData
{
public:
    struct SpectrumBands
    {
        enum {
            count = 1024
        };

        SpectrumBands()
        {
            for (int i = 0; i < count; ++i)
            {
                bandsPeak[i] = 0.0f;
                bandsHold[i] = 0.0f;
            }
            mindB = 0.0f;
            maxdB = 0.0f;
            minFreq = 0.0f;
            maxFreq = 0.0f;
            freqRes = 0.0f;
        };
        
        float   bandsPeak[count];
        float   bandsHold[count];
        float   mindB;
        float   maxdB;
        float   minFreq;
        float   maxFreq;
        float   freqRes;
    };
    
public:
    ProcessorSpectrumData();
    ~ProcessorSpectrumData();
    
    void SetSpectrum(unsigned long channel, SpectrumBands spectrum);
    const SpectrumBands& GetSpectrum(unsigned long channel);
    
    void SetChannelCount(unsigned long count) override;
    unsigned long GetChannelCount() override;
    
private:
    std::map<unsigned long, SpectrumBands>    m_spectrumsMap;

};

}
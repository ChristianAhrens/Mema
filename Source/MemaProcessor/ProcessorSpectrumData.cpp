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

#include "ProcessorSpectrumData.h"

namespace Mema
{

ProcessorSpectrumData::ProcessorSpectrumData()
{
    m_type = Spectrum;
}

ProcessorSpectrumData::~ProcessorSpectrumData()
{

}

void ProcessorSpectrumData::SetSpectrum(unsigned long channel, ProcessorSpectrumData::SpectrumBands spectrum)
{
    m_spectrumsMap[channel] = spectrum;
}

const ProcessorSpectrumData::SpectrumBands& ProcessorSpectrumData::GetSpectrum(unsigned long channel)
{
    if(m_spectrumsMap.count(channel) == 0)
        m_spectrumsMap.insert(std::pair<unsigned long, SpectrumBands>(channel, ProcessorSpectrumData::SpectrumBands{}));

    return m_spectrumsMap.at(channel);
}

void ProcessorSpectrumData::SetChannelCount(unsigned long count)
{
    if(GetChannelCount()==count)
        return;
    
    for(unsigned long i=1; i<=count; i++)
    {
        if(!m_spectrumsMap.count(i))
            m_spectrumsMap[i] = SpectrumBands();
    }
}

unsigned long ProcessorSpectrumData::GetChannelCount()
{
    return static_cast<unsigned long>(m_spectrumsMap.size());
}

}
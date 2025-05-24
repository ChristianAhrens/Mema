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

#include "ProcessorLevelData.h"

namespace Mema
{

ProcessorLevelData::ProcessorLevelData()
{
    m_type = Level;
}

ProcessorLevelData::~ProcessorLevelData()
{

}

void ProcessorLevelData::SetLevel(unsigned long channel, ProcessorLevelData::LevelVal level)
{
    m_levelMap[channel] = level;
}

ProcessorLevelData::LevelVal ProcessorLevelData::GetLevel(unsigned long channel)
{
    if(m_levelMap.count(channel))
        return m_levelMap.at(channel);
    else
        return ProcessorLevelData::LevelVal(0, 0, 0);
}

void ProcessorLevelData::SetChannelCount(unsigned long count)
{
    if(GetChannelCount()==count)
        return;
    
    for(unsigned long i=1; i<=count; i++)
    {
        if (!m_levelMap.count(i))
            m_levelMap[i] = LevelVal(0, 0, 0);
    }
}

unsigned long ProcessorLevelData::GetChannelCount()
{
    return static_cast<unsigned long>(m_levelMap.size());
}

}
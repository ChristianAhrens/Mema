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

namespace Mema
{

/** @class AbstractProcessorData
 *  @brief Base class for all data objects exchanged between the audio processor and its analyzers/visualisers.
 */
class AbstractProcessorData
{
public:
    /** @brief Identifies the concrete payload type carried by this data object. */
    enum Type
    {
        Invalid,    ///< Uninitialised or unknown data.
        AudioSignal, ///< Raw audio buffer data.
        Level,      ///< Peak/RMS/hold level metering data.
        Spectrum,   ///< FFT frequency-spectrum data.
    };

    AbstractProcessorData();
    virtual ~AbstractProcessorData();

    /** @brief Returns the concrete type of this data object. */
    Type GetDataType();

    /** @brief Sets the number of audio channels this data object covers. */
    virtual void SetChannelCount(unsigned long count) = 0;
    /** @brief Returns the number of audio channels this data object covers. */
    virtual unsigned long GetChannelCount() = 0;
    
protected:
    Type    m_type;
};

}
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

/** @class MemaServiceData @brief Compile-time constants for the Mema multicast service discovery protocol. */
struct ServiceData
{
    /** @brief Returns the base string for building service type UIDs. */
	static juce::String getServiceTypeUIDBase();
    /** @brief Returns the UID for the Mema master (server) service. */
	static juce::String getMasterServiceTypeUID();
    /** @brief Returns the UID for the Mema.Mo monitor service. */
	static juce::String getMonitorServiceTypeUID();
    /** @brief Returns the UID for the Mema.Re remote-control service. */
	static juce::String getRemoteServiceTypeUID();
    /** @brief Returns the human-readable service description string. */
	static juce::String getServiceDescription();
    /** @brief Returns the UDP port used for multicast service announcements. */
	static int getBroadcastPort();
    /** @brief Returns the TCP port used for client connections (55668). */
	static int getConnectionPort();

private:
	static constexpr int broadcastPort = 55667;
	static constexpr int connectionPort = 55668;
};

} // namespace Mema

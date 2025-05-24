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

#include "MemaServiceData.h"

namespace Mema
{

juce::String ServiceData::getServiceTypeUID()
{
	return "Mema.MonitorData";
}

juce::String ServiceData::getServiceDescription()
{
	return juce::JUCEApplication::getInstance()->getApplicationName() 
+ "." + juce::JUCEApplication::getInstance()->getApplicationVersion() 
+ "@" + juce::SystemStats::getComputerName();
}

int ServiceData::getBroadcastPort()
{
	return broadcastPort;
}

int ServiceData::getConnectionPort()
{
	return connectionPort;
}

} // namespace Mema

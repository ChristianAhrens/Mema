/* Copyright (c) 2026, Christian Ahrens
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

// Coverage for Mema::ServiceData -- the compile-time constants identifying the
// Mema multicast service discovery protocol (used by Mema.Mo to find Mema
// instances on the network). Pure static string/int constants, no dependencies.
//
// NOTE: ServiceData::getServiceDescription() is declared in MemaServiceData.h but
// never defined anywhere in the codebase, and nothing calls it (every real call
// site uses the unrelated JUCEAppBasics::ServiceTopologyManager::getServiceDescription()
// instead). It is intentionally not exercised here -- referencing it would fail to
// link. Worth removing as dead API in a cleanup pass.

#include <JuceHeader.h>
#include <MemaProcessor/MemaServiceData.h>

using namespace Mema;

class MemaServiceDataTest : public juce::UnitTest
{
public:
    MemaServiceDataTest() : juce::UnitTest ("MemaServiceData", "Mema.Mo") {}

    void runTest() override
    {
        beginTest ("Service type UIDs are built from the common base and are distinct per role");

        auto base = ServiceData::getServiceTypeUIDBase();
        auto master = ServiceData::getMasterServiceTypeUID();
        auto monitor = ServiceData::getMonitorServiceTypeUID();
        auto remote = ServiceData::getRemoteServiceTypeUID();

        expect (master.startsWith (base));
        expect (monitor.startsWith (base));
        expect (remote.startsWith (base));

        expect (master != monitor, "master and monitor service UIDs must be distinct");
        expect (master != remote, "master and remote service UIDs must be distinct");
        expect (monitor != remote, "monitor and remote service UIDs must be distinct");

        beginTest ("Broadcast and connection ports are distinct, valid, non-privileged ports");

        auto broadcastPort = ServiceData::getBroadcastPort();
        auto connectionPort = ServiceData::getConnectionPort();

        expect (broadcastPort != connectionPort, "broadcast and connection must use different ports");
        expect (broadcastPort > 1024 && broadcastPort < 65536, "broadcast port should be a valid non-privileged port");
        expect (connectionPort > 1024 && connectionPort < 65536, "connection port should be a valid non-privileged port");
    }
};

static MemaServiceDataTest memaServiceDataTest;

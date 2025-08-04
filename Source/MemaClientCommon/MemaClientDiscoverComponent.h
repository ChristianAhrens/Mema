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

/**
 * JUCEs' AvailableServiceList has the used listening socket in non-reuse mode, therefor onyl one of our MemaClients can listen on a single machine at a time
 * (applies for JUCE 8.0.8 and is only a problem on UNIX systems)
 * To overcome this, the PortReuseAvailableServiceList is a code clone of JUCEs' AvailableServiceList, with only a single adapted line of code - adding a call
 * to socket.setEnablePortReuse(true) in the constructor. As soon as the JUCE implemenation is changed, we should roll back to the original implementation if possible.
 * See e.g.: https://forum.juce.com/t/networkservicediscovery-multiple-listeners-on-same-machine/66306
 */
struct PortReuseAvailableServiceList : private juce::Thread,
    private juce::AsyncUpdater
{
    /** Creates an AvailableServiceList that will bind to the given port number and watch
        the network for Advertisers broadcasting the given service type.

        This will only detect broadcasts from an Advertiser object with a matching
        serviceTypeUID value, and where the broadcastPort matches.
    */
    PortReuseAvailableServiceList(const String& serviceTypeUID, int broadcastPort);

    /** Destructor */
    ~PortReuseAvailableServiceList() override;

    /** A lambda that can be set to receive a callback when the list changes */
    std::function<void()> onChange;

    /** Returns a list of the currently known services. */
    std::vector<juce::NetworkServiceDiscovery::Service> getServices() const;

private:
    juce::DatagramSocket socket{ true };
    juce::String serviceTypeUID;
    juce::CriticalSection listLock;
    std::vector<juce::NetworkServiceDiscovery::Service> services;

    void run() override;
    void handleAsyncUpdate() override;
    void handleMessage(const juce::XmlElement&);
    void handleMessage(const juce::NetworkServiceDiscovery::Service&);
    void removeTimedOutServices();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PortReuseAvailableServiceList)
};


class MemaClientDiscoverComponent :   public juce::Component
{
public:
    MemaClientDiscoverComponent();
    ~MemaClientDiscoverComponent() override;

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;

    void setupServiceDiscovery();
    void resetServices();

    std::vector<NetworkServiceDiscovery::Service> getAvailableServices();

    //==============================================================================
    std::function<void(const juce::NetworkServiceDiscovery::Service&)> onServiceSelected;

private:
    //==============================================================================
    void setDiscoveredServices(const std::vector<juce::NetworkServiceDiscovery::Service>& services);
    
    //==============================================================================
    std::unique_ptr<juce::Label>                        m_discoveredServicesLabel;
    std::unique_ptr<juce::ComboBox>                     m_discoveredServicesSelection;
    std::vector<juce::NetworkServiceDiscovery::Service> m_discoveredServices;

    std::unique_ptr<PortReuseAvailableServiceList>    m_availableServices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaClientDiscoverComponent)
};


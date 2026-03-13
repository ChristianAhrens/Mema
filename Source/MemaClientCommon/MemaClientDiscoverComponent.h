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

#include <ServiceTopologyManager.h>

namespace JUCEAppBasics
{
    class ServiceTopologyTreeView;
}

/** @class MemaClientDiscoverComponent @brief Service-discovery panel that listens for Mema multicast announcements and lists available instances. */
class MemaClientDiscoverComponent :   public juce::Component
{
public:
    MemaClientDiscoverComponent();
    ~MemaClientDiscoverComponent() override;

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;

    void setupServiceDiscovery(const juce::String& serviceTypeUIDBase, const juce::String& serviceTypeUID);
    void resetServices();

    std::vector<JUCEAppBasics::SessionMasterAwareService> getAvailableServices();

    void setMasterServiceDescription(const juce::String& masterServiceDescription);

    //==============================================================================
    std::function<void(const JUCEAppBasics::SessionMasterAwareService&)> onServiceSelected;

private:
    //==============================================================================
    void setDiscoveredServiceTopology(const JUCEAppBasics::SessionServiceTopology& topology);
    
    //==============================================================================
    std::unique_ptr<juce::Label>                            m_discoveredTopologyLabel;
    std::unique_ptr<JUCEAppBasics::ServiceTopologyTreeView> m_discoveredTopologyTreeView;
    std::unique_ptr<juce::TextButton>                       m_selectServiceButton;

    std::unique_ptr<JUCEAppBasics::ServiceTopologyManager>  m_serviceTopologyManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaClientDiscoverComponent)
};


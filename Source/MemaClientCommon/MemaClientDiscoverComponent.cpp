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

#include "MemaClientDiscoverComponent.h"

#include <MemaProcessor/MemaServiceData.h>

#include <CustomLookAndFeel.h>
#include <ServiceTopologyTreeView.h>


MemaClientDiscoverComponent::MemaClientDiscoverComponent()
    : juce::Component()
{
    m_discoveredTopologyLabel = std::make_unique<juce::Label>("TopologyLabel", "Available Mema sessions:");
    addAndMakeVisible(m_discoveredTopologyLabel.get());

    m_discoveredTopologyTreeView = std::make_unique<JUCEAppBasics::ServiceTopologyTreeView>(true);
    addAndMakeVisible(m_discoveredTopologyTreeView.get());

    m_selectServiceButton = std::make_unique<juce::TextButton>("Join session", "Join the selected Mema session.");
    m_selectServiceButton->onClick = [=]() {
        if (onServiceSelected && m_discoveredTopologyTreeView)
        {
            auto item = dynamic_cast<JUCEAppBasics::MasterServiceTreeViewItem*>(m_discoveredTopologyTreeView->getSelectedItem(0));
            if (nullptr != item)
                onServiceSelected(item->getServiceInfo());
        }
    };
    addAndMakeVisible(m_selectServiceButton.get());
}

MemaClientDiscoverComponent::~MemaClientDiscoverComponent()
{
}

void MemaClientDiscoverComponent::paint(Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::ColourIds::backgroundColourId));
}

void MemaClientDiscoverComponent::resized()
{
    auto labelHeight = 35;
    auto buttonHeight = 35;
    auto margin = 4;
    auto maxDiscoveryElmsWidth = 450;
    auto maxDiscoveryElmsHeight = 350;

    auto bounds = getLocalBounds().reduced(2*margin);
    if (bounds.getWidth() > maxDiscoveryElmsWidth)
    {
        auto hmargin = int((bounds.getWidth() - maxDiscoveryElmsWidth) * 0.5f);
        bounds.removeFromLeft(hmargin);
        bounds.removeFromRight(hmargin);
    }
    if (bounds.getHeight() > maxDiscoveryElmsHeight)
    {
        auto vmargin = int((bounds.getHeight() - maxDiscoveryElmsHeight) * 0.5f);
        bounds.removeFromTop(vmargin);
        bounds.removeFromBottom(vmargin);
    }

    if (m_selectServiceButton)
        m_selectServiceButton->setBounds(bounds.removeFromBottom(buttonHeight));
    bounds.removeFromBottom(margin);
    if (m_discoveredTopologyLabel)
        m_discoveredTopologyLabel->setBounds(bounds.removeFromTop(labelHeight));
    if (m_discoveredTopologyTreeView)
        m_discoveredTopologyTreeView->setBounds(bounds);
}

void MemaClientDiscoverComponent::lookAndFeelChanged()
{
    getLookAndFeel().setColour(
        TreeView::ColourIds::selectedItemBackgroundColourId, 
        getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::MeteringRmsColourId));
}

void MemaClientDiscoverComponent::resetServices()
{
    setMasterServiceDescription("");
}

void MemaClientDiscoverComponent::setupServiceDiscovery(const juce::String& serviceTypeUIDBase, const juce::String& serviceTypeUID)
{
    // scope to autodestruct testSocket
    {
        auto testSocket = std::make_unique<juce::DatagramSocket>();
        testSocket->setEnablePortReuse(true);
        if (!testSocket->bindToPort(Mema::ServiceData::getBroadcastPort()))
        {
            auto conflictTitle = "Service discovery error";
            auto conflictInfo = "Unable to discover Mema instances\nas the discovery broadcast port cannot be used.";
            juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::WarningIcon, conflictTitle, conflictInfo, "Retry", "Quit", nullptr, juce::ModalCallbackFunction::create([this, serviceTypeUIDBase, serviceTypeUID](int result) {
                if (1 == result)
                    setupServiceDiscovery(serviceTypeUIDBase, serviceTypeUID);
                else
                    juce::JUCEApplication::getInstance()->quit();
            }));
        }
    }

    m_serviceTopologyManager = std::make_unique<JUCEAppBasics::ServiceTopologyManager>(
        serviceTypeUIDBase, serviceTypeUID,
        Mema::ServiceData::getServiceDescription(),
        "",
        Mema::ServiceData::getBroadcastPort(),
        Mema::ServiceData::getConnectionPort());

    m_serviceTopologyManager->onDiscoveredTopologyChanged = [=]() {
        if (m_serviceTopologyManager)
            setDiscoveredServiceTopology(m_serviceTopologyManager->getDiscoveredServiceTopology());
    };
}

std::vector<JUCEAppBasics::SessionMasterAwareService> MemaClientDiscoverComponent::getAvailableServices()
{
    std::vector<JUCEAppBasics::SessionMasterAwareService> services;
    services.reserve(m_serviceTopologyManager->getDiscoveredServiceTopology().size());
    for (auto const& serviceKV : m_serviceTopologyManager->getDiscoveredServiceTopology())
        services.push_back(serviceKV.first);
    return services;
}
void MemaClientDiscoverComponent::setMasterServiceDescription(const juce::String& masterServiceDescription)
{
    if (m_serviceTopologyManager)
        m_serviceTopologyManager->setSessionMasterServiceDescription(masterServiceDescription);
}

void MemaClientDiscoverComponent::setDiscoveredServiceTopology(const JUCEAppBasics::SessionServiceTopology& topology)
{
    if (m_discoveredTopologyTreeView)
        m_discoveredTopologyTreeView->setServiceTopology(topology);
}


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



 //==============================================================================
PortReuseAvailableServiceList::PortReuseAvailableServiceList(const juce::String& serviceType, int broadcastPort)
    : juce::Thread(SystemStats::getJUCEVersion() + ": Discovery_listen"), serviceTypeUID(serviceType)
{
#if JUCE_ANDROID
    acquireMulticastLock();
#endif

    socket.setEnablePortReuse(true);
    socket.bindToPort(broadcastPort);
    startThread(juce::Thread::Priority::background);
}

PortReuseAvailableServiceList::~PortReuseAvailableServiceList()
{
    socket.shutdown();
    stopThread(2000);

#if JUCE_ANDROID
    releaseMulticastLock();
#endif
}

void PortReuseAvailableServiceList::run()
{
    while (!threadShouldExit())
    {
        if (socket.waitUntilReady(true, 200) == 1)
        {
            char buffer[1024];
            auto bytesRead = socket.read(buffer, sizeof(buffer) - 1, false);

            if (bytesRead > 10)
                if (auto xml = parseXML(juce::String(juce::CharPointer_UTF8(buffer),
                    juce::CharPointer_UTF8(buffer + bytesRead))))
                    if (xml->hasTagName(serviceTypeUID))
                        handleMessage(*xml);
        }

        removeTimedOutServices();
    }
}

std::vector<juce::NetworkServiceDiscovery::Service> PortReuseAvailableServiceList::getServices() const
{
    const juce::ScopedLock sl(listLock);
    auto listCopy = services;
    return listCopy;
}

void PortReuseAvailableServiceList::handleAsyncUpdate()
{
    juce::NullCheckedInvocation::invoke(onChange);
}

void PortReuseAvailableServiceList::handleMessage(const XmlElement& xml)
{
    juce::NetworkServiceDiscovery::Service service;
    service.instanceID = xml.getStringAttribute("id");

    if (service.instanceID.trim().isNotEmpty())
    {
        service.description = xml.getStringAttribute("name");
        service.address = juce::IPAddress(xml.getStringAttribute("address"));
        service.port = xml.getIntAttribute("port");
        service.lastSeen = juce::Time::getCurrentTime();

        handleMessage(service);
    }
}

static void sortServiceList(std::vector<juce::NetworkServiceDiscovery::Service>& services)
{
    auto compareServices = [](const juce::NetworkServiceDiscovery::Service& s1,
        const juce::NetworkServiceDiscovery::Service& s2)
        {
            return s1.instanceID < s2.instanceID;
        };

    std::sort(services.begin(), services.end(), compareServices);
}

void PortReuseAvailableServiceList::handleMessage(const juce::NetworkServiceDiscovery::Service& service)
{
    const juce::ScopedLock sl(listLock);

    for (auto& s : services)
    {
        if (s.instanceID == service.instanceID)
        {
            if (s.description != service.description
                || s.address != service.address
                || s.port != service.port)
            {
                s = service;
                triggerAsyncUpdate();
            }

            s.lastSeen = service.lastSeen;
            return;
        }
    }

    services.push_back(service);
    sortServiceList(services);
    triggerAsyncUpdate();
}

void PortReuseAvailableServiceList::removeTimedOutServices()
{
    const double timeoutSeconds = 5.0;
    auto oldestAllowedTime = juce::Time::getCurrentTime() - juce::RelativeTime::seconds(timeoutSeconds);

    const juce::ScopedLock sl(listLock);

    auto oldEnd = std::end(services);
    auto newEnd = std::remove_if(std::begin(services), oldEnd,
        [=](const juce::NetworkServiceDiscovery::Service& s) { return s.lastSeen < oldestAllowedTime; });

    if (newEnd != oldEnd)
    {
        services.erase(newEnd, oldEnd);
        triggerAsyncUpdate();
    }
}


MemaClientDiscoverComponent::MemaClientDiscoverComponent()
    : juce::Component()
{
    m_discoveredServicesLabel = std::make_unique<juce::Label>("ServicesLabel", "Available Mema instances:");
    addAndMakeVisible(m_discoveredServicesLabel.get());

    m_discoveredServicesSelection = std::make_unique<juce::ComboBox>("ServicesComboBox");
    m_discoveredServicesSelection->onChange = [=]() {
        auto idx = m_discoveredServicesSelection->getSelectedItemIndex();
        
        if (onServiceSelected && m_discoveredServices.size() > idx)
            onServiceSelected(m_discoveredServices.at(idx));
    };
    m_discoveredServicesSelection->setTextWhenNoChoicesAvailable("Select an instance to connect");
    m_discoveredServicesSelection->setTextWhenNoChoicesAvailable("None");
    addAndMakeVisible(m_discoveredServicesSelection.get());

    setupServiceDiscovery();
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
    auto contentWidth = 250;
    auto contentHeight = 80;
    auto elementHeight = contentHeight / 2;
    auto elementsBounds = juce::Rectangle<int>((getWidth() - contentWidth) / 2, (getHeight() - contentHeight) / 2, contentWidth, contentHeight);

    m_discoveredServicesLabel->setBounds(elementsBounds.removeFromTop(elementHeight));
    m_discoveredServicesSelection->setBounds(elementsBounds);
}

void MemaClientDiscoverComponent::resetServices()
{
    if (m_discoveredServicesSelection)
        m_discoveredServicesSelection->setSelectedId(-1, juce::dontSendNotification);
}

void MemaClientDiscoverComponent::setDiscoveredServices(const std::vector<juce::NetworkServiceDiscovery::Service>& services)
{
    m_discoveredServices = services;

    if (m_discoveredServicesSelection)
        m_discoveredServicesSelection->clear();

    int i = 1;
    for (auto const& service : services)
    {
        m_discoveredServicesSelection->addItem(service.description, i);
        i++;
    }
}

void MemaClientDiscoverComponent::setupServiceDiscovery()
{
    // scope to autodestruct testSocket
    {
        auto testSocket = std::make_unique<juce::DatagramSocket>();
        testSocket->setEnablePortReuse(true);
        if (!testSocket->bindToPort(Mema::ServiceData::getBroadcastPort()))
        {
            auto conflictTitle = "Service discovery error";
            auto conflictInfo = "Unable to discover Mema instances\nas the discovery broadcast port cannot be used.";
            juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::WarningIcon, conflictTitle, conflictInfo, "Retry", "Quit", nullptr, juce::ModalCallbackFunction::create([this](int result) {
                if (1 == result)
                    setupServiceDiscovery();
                else
                    juce::JUCEApplication::getInstance()->quit();
            }));
        }
    }

    m_availableServices = std::make_unique<PortReuseAvailableServiceList>(
        Mema::ServiceData::getServiceTypeUID(),
        Mema::ServiceData::getBroadcastPort());

    m_availableServices->onChange = [=]() {
        setDiscoveredServices(m_availableServices->getServices());
    };
}

std::vector<NetworkServiceDiscovery::Service> MemaClientDiscoverComponent::getAvailableServices()
{
    if (m_availableServices)
        return m_availableServices->getServices();
    else
        return {};
}


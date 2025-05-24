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

#include "MemaDiscoverComponent.h"

MemaDiscoverComponent::MemaDiscoverComponent()
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
}

MemaDiscoverComponent::~MemaDiscoverComponent()
{
}

void MemaDiscoverComponent::paint(Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::ColourIds::backgroundColourId));
}

void MemaDiscoverComponent::resized()
{
    auto contentWidth = 250;
    auto contentHeight = 80;
    auto elementHeight = contentHeight / 2;
    auto elementsBounds = juce::Rectangle<int>((getWidth() - contentWidth) / 2, (getHeight() - contentHeight) / 2, contentWidth, contentHeight);

    m_discoveredServicesLabel->setBounds(elementsBounds.removeFromTop(elementHeight));
    m_discoveredServicesSelection->setBounds(elementsBounds);
}

void MemaDiscoverComponent::setDiscoveredServices(const std::vector<juce::NetworkServiceDiscovery::Service>& services)
{
    m_discoveredServices = services;

    m_discoveredServicesSelection->clear();
    int i = 1;
    for (auto const& service : services)
    {
        m_discoveredServicesSelection->addItem(service.description, i);
        i++;
    }
}

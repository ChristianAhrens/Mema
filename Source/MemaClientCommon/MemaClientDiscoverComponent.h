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


class MemaClientDiscoverComponent :   public juce::Component
{
public:
    MemaClientDiscoverComponent();
    ~MemaClientDiscoverComponent() override;

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==============================================================================
    void setDiscoveredServices(const std::vector<juce::NetworkServiceDiscovery::Service>& services);

    //==============================================================================
    std::function<void(const juce::NetworkServiceDiscovery::Service&)> onServiceSelected;

private:
    std::unique_ptr<juce::Label>                        m_discoveredServicesLabel;
    std::unique_ptr<juce::ComboBox>                     m_discoveredServicesSelection;
    std::vector<juce::NetworkServiceDiscovery::Service> m_discoveredServices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaClientDiscoverComponent)
};


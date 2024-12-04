/* Copyright (c) 2024, Christian Ahrens
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


class MemaMoComponent;
class MemaDiscoverComponent;
class AboutComponent;

class MainComponent :   public juce::Component
{
public:
    enum Status
    {
        Discovering,
        Monitoring
    };

    enum SettingsOptions
    {
        LookAndFeel_First = 1,
        LookAndFeel_FollowHost = LookAndFeel_First,
        LookAndFeel_Dark,
        LookAndFeel_Light,
        LookAndFeel_Last = LookAndFeel_Light,
        OutputVisuType_First,
        OutputVisuType_Meterbridge = OutputVisuType_First,
        OutputVisuType_LRS,
        OutputVisuType_LCRS,
        OutputVisuType_5point0,
        OutputVisuType_5point1,
        OutputVisuType_5point1point2,
        OutputVisuType_7point0,
        OutputVisuType_7point1,
        OutputVisuType_7point1point4,
        OutputVisuType_Last = OutputVisuType_7point1point4
    };

public:
    MainComponent();
    ~MainComponent() override;

    //========================================================================*
    void resized() override;
    void paint(juce::Graphics& g) override;
    void lookAndFeelChanged() override;

    //========================================================================*
    std::function<void(int, bool)> onPaletteStyleChange;

private:
    //========================================================================*
    class InterprocessConnectionImpl : public juce::InterprocessConnection
    {
    public:
        InterprocessConnectionImpl() : juce::InterprocessConnection() {};
        virtual ~InterprocessConnectionImpl() { disconnect(); };

        void connectionMade() override { if (onConnectionMade) onConnectionMade(); };

        void connectionLost() override { if (onConnectionLost) onConnectionLost(); };

        void messageReceived(const MemoryBlock& message) override { if (onMessageReceived) onMessageReceived(message); };

        std::function<void()>                   onConnectionMade;
        std::function<void()>                   onConnectionLost;
        std::function<void(const MemoryBlock&)> onMessageReceived;

    private:

    };

    //========================================================================*
    void handleSettingsMenuResult(int selectedId);
    void handleSettingsLookAndFeelMenuResult(int selectedId);
    void handleSettingsOutputVisuTypeMenuResult(int selectedId);

    //========================================================================*
    std::unique_ptr<juce::NetworkServiceDiscovery::AvailableServiceList>    m_availableServices;
    std::unique_ptr<InterprocessConnectionImpl>                             m_networkConnection;

    std::unique_ptr<MemaMoComponent>                                        m_monitorComponent;
    std::unique_ptr<MemaDiscoverComponent>                                  m_discoverComponent;

    std::unique_ptr<juce::DrawableButton>                                   m_settingsButton;
    std::map<int, std::pair<std::string, int>>                              m_settingsItems;
    int                                                                     m_settingsHostLookAndFeelId = -1;

    std::unique_ptr<juce::DrawableButton>                                   m_disconnectButton;

    std::unique_ptr<juce::DrawableButton>                                   m_aboutButton;
    std::unique_ptr<AboutComponent>                                         m_aboutComponent;

    Status m_currentStatus = Status::Discovering;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


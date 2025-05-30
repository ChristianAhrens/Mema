/* Copyright (c) 2025, Christian Ahrens
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

#include "MemaReAppConfiguration.h"


class MemaReComponent;
class MemaClientDiscoverComponent;
class MemaClientConnectingComponent;
class AboutComponent;

class MainComponent :   public juce::Component,
                        public juce::Timer,
                        public MemaReAppConfiguration::Dumper,
                        public MemaReAppConfiguration::Watcher
{
public:
    enum Status
    {
        Discovering,
        Connecting,
        Monitoring,
    };

    enum MemaReSettingsOption
    {
        LookAndFeel_First = 1,
        LookAndFeel_FollowHost = LookAndFeel_First,
        LookAndFeel_Dark,
        LookAndFeel_Light,
        LookAndFeel_Last = LookAndFeel_Light,
        OutputPanningType_First,
        OutputPanningType_RawChannels = OutputPanningType_First,
        OutputPanningType_LRS,
        OutputPanningType_LCRS,
        OutputPanningType_5point0,
        OutputPanningType_5point1,
        OutputPanningType_5point1point2,
        OutputPanningType_7point0,
        OutputPanningType_7point1,
        OutputPanningType_7point1point4,
        OutputPanningType_9point1point6,
        OutputPanningType_Last = OutputPanningType_9point1point6,
        PanningColour_First,
        PanningColour_Green = PanningColour_First,
        PanningColour_Red,
        PanningColour_Blue,
        PanningColour_Pink,
        PanningColour_Last = PanningColour_Pink
    };

public:
    MainComponent();
    ~MainComponent() override;

    void applySettingsOption(const MemaReSettingsOption& option);

    //==============================================================================
    void resized() override;
    void paint(juce::Graphics& g) override;
    void lookAndFeelChanged() override;

    void timerCallback() override;

    //==============================================================================
    void performConfigurationDump() override;
    void onConfigUpdated() override;

    //==============================================================================
    std::function<void(int, bool)> onPaletteStyleChange;

private:
    //==============================================================================
    class InterprocessConnectionImpl : public juce::InterprocessConnection
    {
    public:
        InterprocessConnectionImpl() : juce::InterprocessConnection() {};
        virtual ~InterprocessConnectionImpl() { disconnect(); };

        void connectionMade() override { if (onConnectionMade) onConnectionMade(); };

        void connectionLost() override { if (onConnectionLost) onConnectionLost(); };

        void messageReceived(const MemoryBlock& message) override { if (onMessageReceived) onMessageReceived(message); };

        bool ConnectToSocket(const juce::String& hostName, int portNumber) {
            m_hostName = hostName;
            m_portNumber = portNumber;
            return juce::InterprocessConnection::connectToSocket(hostName, portNumber, 3000);
        };
        
        bool RetryConnectToSocket() { 
            disconnect();
            return connectToSocket(m_hostName, m_portNumber, 3000);
        };

        std::function<void()>                   onConnectionMade;
        std::function<void()>                   onConnectionLost;
        std::function<void(const MemoryBlock&)> onMessageReceived;

    private:
        juce::String m_hostName;
        int m_portNumber = 0;
    };

    //==============================================================================
    void handleSettingsMenuResult(int selectedId);
    void handleSettingsLookAndFeelMenuResult(int selectedId);
    void handleSettingsOutputPanningTypeMenuResult(int selectedId);
    void handleSettingsPanningColourMenuResult(int selectedId);

    void setPanningColour(const juce::Colour& meteringColour);
    void applyPanningColour();

    void setStatus(const Status& s);
    const Status getStatus();

    void connectToMema();

    //==============================================================================
    std::unique_ptr<juce::NetworkServiceDiscovery::AvailableServiceList>    m_availableServices;
    juce::NetworkServiceDiscovery::Service                                  m_selectedService;
    std::unique_ptr<InterprocessConnectionImpl>                             m_networkConnection;

    std::unique_ptr<MemaReComponent>                                        m_remoteComponent;
    std::unique_ptr<MemaClientDiscoverComponent>                            m_discoverComponent;
    std::unique_ptr<MemaClientConnectingComponent>                          m_connectingComponent;

    std::unique_ptr<juce::DrawableButton>                                   m_settingsButton;
    std::map<int, std::pair<std::string, int>>                              m_settingsItems;
    int                                                                     m_settingsHostLookAndFeelId = -1;

    std::unique_ptr<juce::DrawableButton>                                   m_disconnectButton;

    std::unique_ptr<juce::DrawableButton>                                   m_aboutButton;
    std::unique_ptr<AboutComponent>                                         m_aboutComponent;

    Status                                                                  m_currentStatus = Status::Discovering;

    juce::Colour                                                            m_panningColour = juce::Colours::forestgreen;

    std::unique_ptr<MemaReAppConfiguration>                                 m_config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


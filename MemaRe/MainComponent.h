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

#include "MemaClientCommon/MemaClientControlComponentBase.h"

#include <ServiceTopologyManager.h>


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
        ControlFormat_First,
        ControlFormat_RawChannels = ControlFormat_First,
        ControlFormat_PanningType_LRS,
        ControlFormat_PanningType_LCRS,
        ControlFormat_PanningType_5point0,
        ControlFormat_PanningType_5point1,
        ControlFormat_PanningType_5point1point2,
        ControlFormat_PanningType_7point0,
        ControlFormat_PanningType_7point1,
        ControlFormat_PanningType_7point1point4,
        ControlFormat_PanningType_9point1point6,
        ControlFormat_PanningType_Quadrophonic,
        ControlFormat_PluginParameterControl,
        ControlFormat_Last = ControlFormat_PluginParameterControl,
        ControlColour_First,
        ControlColour_Green = ControlColour_First,
        ControlColour_Red,
        ControlColour_Blue,
        ControlColour_Pink,
        ControlColour_Last = ControlColour_Pink,
        ControlsSize_First,
        ControlsSize_S = ControlsSize_First,
        ControlsSize_M,
        ControlsSize_L,
        ControlsSize_Last = ControlsSize_L,
        ExternalControl,
        FullscreenWindowMode,
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

    bool keyPressed(const juce::KeyPress& key) override;

    //==============================================================================
    void performConfigurationDump() override;
    void onConfigUpdated() override;

    //==============================================================================
    bool isFullscreenEnabled();

    //==============================================================================
    std::function<void(int, bool)> onPaletteStyleChange;

    //==============================================================================
    std::function<void(bool)> onSetFullscreenWindow;

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
    void handleSettingsControlFormatMenuResult(int selectedId);
    void handleSettingsControlColourMenuResult(int selectedId);
    void handleSettingsControlsSizeMenuResult(int selectedId);
    void handleSettingsFullscreenModeToggleResult();
    void showExternalControlSettings();

    void setControlColour(const juce::Colour& meteringColour);
    void applyControlColour();

    void setControlsSize(const Mema::MemaClientControlComponentBase::ControlsSize& controlsSize);

    void toggleFullscreenMode();

    void setStatus(const Status& s);
    const Status getStatus();

    void connectToMema();

    //==============================================================================
    JUCEAppBasics::SessionMasterAwareService        m_selectedService;
    std::unique_ptr<InterprocessConnectionImpl>     m_networkConnection;

    std::unique_ptr<MemaReComponent>                m_remoteComponent;
    std::unique_ptr<MemaClientDiscoverComponent>    m_discoverComponent;
    std::unique_ptr<MemaClientConnectingComponent>  m_connectingComponent;

    std::unique_ptr<juce::DrawableButton>           m_settingsButton;
    std::map<int, std::pair<std::string, int>>      m_settingsItems;
    int                                             m_settingsHostLookAndFeelId = -1;

    std::unique_ptr<juce::DrawableButton>           m_disconnectButton;

    std::unique_ptr<juce::DrawableButton>           m_aboutButton;
    std::unique_ptr<AboutComponent>                 m_aboutComponent;

    std::unique_ptr<juce::AlertWindow>              m_messageBox;

    Status                                          m_currentStatus = Status::Discovering;

    juce::Colour                                    m_controlColour = juce::Colours::forestgreen;

    std::unique_ptr<MemaReAppConfiguration>         m_config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


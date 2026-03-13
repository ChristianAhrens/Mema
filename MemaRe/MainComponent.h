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

/**
 * @class MainComponent
 * @brief Top-level application component for **Mema.Re** (MenubarMatrixRemote).
 *
 * MainComponent drives the three-phase connection state machine of Mema.Re and owns
 * all child components that are active in each phase:
 *
 * | Phase | Active child | Description |
 * |-------|--------------|-------------|
 * | Discovering | `MemaClientDiscoverComponent` | Listens for multicast service announcements from Mema instances on the local network. |
 * | Connecting  | `MemaClientConnectingComponent` | Establishes the TCP connection to the selected Mema server. |
 * | Controlling | `MemaReComponent` | Sends user control gestures to Mema and keeps local state in sync with incoming snapshots. |
 *
 * It also owns the TCP socket (`InterprocessConnectionImpl`), exposes a settings menu
 * for choosing look-and-feel, control format, accent colour, control size, and ADM-OSC
 * settings, and persists all choices via `MemaReAppConfiguration`.
 *
 * ## Role in the Mema tool suite
 * Mema.Re is the **bidirectional remote-control companion** to the Mema audio-matrix server.
 * - On connect, Mema sends a full `ControlParametersMessage` state snapshot so Mema.Re can
 *   initialise its UI to the current server state.
 * - User interactions produce updated `ControlParametersMessage` or `PluginParameterValueMessage`
 *   payloads that are sent back to Mema over the same TCP connection.
 * - Optionally, an external spatial-audio controller (e.g. Grapes) can send ADM-OSC UDP
 *   packets to `ADMOSController`, which feeds position/mute updates into the 2-D panning view.
 *
 * @see MemaReComponent         — the active remote-control panel.
 * @see MemaReAppConfiguration  — XML persistence of settings.
 */
class MainComponent :   public juce::Component,
                        public juce::Timer,
                        public MemaReAppConfiguration::Dumper,
                        public MemaReAppConfiguration::Watcher
{
public:
    /** @brief Connection/application phase driven by the TCP session lifecycle. */
    enum Status
    {
        Discovering,    ///< Searching for Mema instances via multicast.
        Connecting,     ///< TCP handshake in progress.
        Monitoring,     ///< Connected and actively sending/receiving control data.
    };

    /** @brief Identifiers for all user-configurable settings exposed via the settings popup menu. */
    enum MemaReSettingsOption
    {
        LookAndFeel_First = 1,
        LookAndFeel_FollowHost = LookAndFeel_First,         ///< Match the host OS appearance.
        LookAndFeel_Dark,                                   ///< Force dark look-and-feel.
        LookAndFeel_Light,                                  ///< Force light look-and-feel.
        LookAndFeel_Last = LookAndFeel_Light,
        ControlFormat_First,
        ControlFormat_RawChannels = ControlFormat_First,    ///< Faderbank crosspoint control (raw input × output).
        ControlFormat_PanningType_LRS,                      ///< 2-D spatial panning — LRS 3-channel.
        ControlFormat_PanningType_LCRS,                     ///< 2-D spatial panning — LCRS 4-channel.
        ControlFormat_PanningType_5point0,                  ///< 2-D spatial panning — 5.0 surround.
        ControlFormat_PanningType_5point1,                  ///< 2-D spatial panning — 5.1 surround.
        ControlFormat_PanningType_5point1point2,            ///< 2-D spatial panning — 5.1.2 with height.
        ControlFormat_PanningType_7point0,                  ///< 2-D spatial panning — 7.0 surround.
        ControlFormat_PanningType_7point1,                  ///< 2-D spatial panning — 7.1 surround.
        ControlFormat_PanningType_7point1point4,            ///< 2-D spatial panning — 7.1.4 Dolby Atmos.
        ControlFormat_PanningType_9point1point6,            ///< 2-D spatial panning — 9.1.6 ATMOS full-3D.
        ControlFormat_PanningType_Quadrophonic,             ///< 2-D spatial panning — classic 4-channel quad.
        ControlFormat_PluginParameterControl,               ///< Plugin-parameter control panel.
        ControlFormat_Last = ControlFormat_PluginParameterControl,
        ControlColour_First,
        ControlColour_Green = ControlColour_First,  ///< Green accent colour (default).
        ControlColour_Red,                          ///< Red accent colour.
        ControlColour_Blue,                         ///< Blue accent colour.
        ControlColour_Pink,                         ///< Pink accent colour.
        ControlColour_Laser,                        ///< High-visibility laser-green accent colour.
        ControlColour_Last = ControlColour_Laser,
        ControlsSize_First,
        ControlsSize_S = ControlsSize_First,        ///< Small control elements.
        ControlsSize_M,                             ///< Medium control elements.
        ControlsSize_L,                             ///< Large control elements (good for touch screens).
        ControlsSize_Last = ControlsSize_L,
        ExternalControl,                            ///< Opens the ADM-OSC external-control settings dialog.
        FullscreenWindowMode,                       ///< Toggle between popup and fullscreen window mode.
    };

public:
    MainComponent();
    ~MainComponent() override;

    /** @brief Applies a settings menu selection, updating look-and-feel, control format, colour, size, or ADM-OSC settings. */
    void applySettingsOption(const MemaReSettingsOption& option);

    //==============================================================================
    /** @brief Lays out the active child component to fill the window. */
    void resized() override;
    /** @brief Paints the background and any status overlay. */
    void paint(juce::Graphics& g) override;
    /** @brief Propagates a look-and-feel change to all owned child components. */
    void lookAndFeelChanged() override;

    /** @brief Periodic callback used to retry TCP connections and poll status. */
    void timerCallback() override;

    /** @brief Handles keyboard shortcuts (e.g. Escape to disconnect). */
    bool keyPressed(const juce::KeyPress& key) override;

    //==============================================================================
    /** @brief Serialises the current configuration to the XML file on disk. */
    void performConfigurationDump() override;
    /** @brief Reacts to external configuration changes. */
    void onConfigUpdated() override;

    //==============================================================================
    /** @brief Returns whether the window is currently displayed in fullscreen mode. */
    bool isFullscreenEnabled();

    //==============================================================================
    std::function<void(int, bool)> onPaletteStyleChange;    ///< Called when the user changes the look-and-feel or accent colour.

    //==============================================================================
    std::function<void(bool)> onSetFullscreenWindow;        ///< Called to request a fullscreen/windowed transition from the application shell.

private:
    /**
     * @class InterprocessConnectionImpl
     * @brief Thin wrapper around juce::InterprocessConnection that forwards events via std::function callbacks.
     *
     * Encapsulates the TCP client socket used to exchange control messages with the Mema server.
     * All three JUCE interprocess events are forwarded to lambdas set by `MainComponent` so that
     * socket logic stays decoupled from UI state management.
     */
    class InterprocessConnectionImpl : public juce::InterprocessConnection
    {
    public:
        InterprocessConnectionImpl() : juce::InterprocessConnection() {};
        virtual ~InterprocessConnectionImpl() { disconnect(); };

        /** @brief Delegates to onConnectionMade when the TCP handshake completes. */
        void connectionMade() override { if (onConnectionMade) onConnectionMade(); };

        /** @brief Delegates to onConnectionLost when the TCP connection drops. */
        void connectionLost() override { if (onConnectionLost) onConnectionLost(); };

        /** @brief Delegates to onMessageReceived when a complete framed message arrives. */
        void messageReceived(const MemoryBlock& message) override { if (onMessageReceived) onMessageReceived(message); };

        /** @brief Stores the target address and calls connectToSocket with a 3-second timeout. */
        bool ConnectToSocket(const juce::String& hostName, int portNumber) {
            m_hostName = hostName;
            m_portNumber = portNumber;
            return juce::InterprocessConnection::connectToSocket(hostName, portNumber, 3000);
        };

        /** @brief Disconnects and immediately retries using the last stored host/port. */
        bool RetryConnectToSocket() {
            disconnect();
            return connectToSocket(m_hostName, m_portNumber, 3000);
        };

        std::function<void()>                   onConnectionMade;       ///< Fired on the JUCE IPC thread when the TCP session is established.
        std::function<void()>                   onConnectionLost;       ///< Fired on the JUCE IPC thread when the TCP session is dropped.
        std::function<void(const MemoryBlock&)> onMessageReceived;      ///< Fired for every complete framed message received from Mema.

    private:
        juce::String m_hostName;    ///< Hostname or IP address of the last connection target.
        int m_portNumber = 0;       ///< Port of the last connection target.
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
    JUCEAppBasics::SessionMasterAwareService        m_selectedService;          ///< Multicast service descriptor of the Mema instance chosen by the user.
    std::unique_ptr<InterprocessConnectionImpl>     m_networkConnection;        ///< Active TCP client socket (null while Discovering).

    std::unique_ptr<MemaReComponent>                m_remoteComponent;          ///< Active remote-control panel (Controlling phase).
    std::unique_ptr<MemaClientDiscoverComponent>    m_discoverComponent;        ///< Service-discovery panel (Discovering phase).
    std::unique_ptr<MemaClientConnectingComponent>  m_connectingComponent;      ///< Connection-progress panel (Connecting phase).

    std::unique_ptr<juce::DrawableButton>           m_settingsButton;           ///< Gear icon that opens the settings popup menu.
    std::map<int, std::pair<std::string, int>>      m_settingsItems;            ///< Mapping from menu item ID to {label, MemaReSettingsOption}.
    int                                             m_settingsHostLookAndFeelId = -1; ///< Menu ID assigned to the "Follow host" look-and-feel entry.

    std::unique_ptr<juce::DrawableButton>           m_disconnectButton;         ///< Button to drop the current TCP connection and return to Discovering.

    std::unique_ptr<juce::DrawableButton>           m_aboutButton;              ///< Button that shows the about popup.
    std::unique_ptr<AboutComponent>                 m_aboutComponent;           ///< The about popup component.

    std::unique_ptr<juce::AlertWindow>              m_messageBox;               ///< General-purpose alert dialog (e.g. ADM-OSC settings errors).

    Status                                          m_currentStatus = Status::Discovering; ///< Current state-machine phase.

    juce::Colour                                    m_controlColour = juce::Colours::forestgreen; ///< Active accent colour for control elements.

    std::unique_ptr<MemaReAppConfiguration>         m_config;                   ///< XML configuration manager.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


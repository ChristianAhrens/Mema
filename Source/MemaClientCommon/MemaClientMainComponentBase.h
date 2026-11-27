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

#include <ServiceTopologyManager.h>


class MemaClientDiscoverComponent;
class MemaClientConnectingComponent;

/**
 * @class MemaClientMainComponentBase
 * @brief Connection state machine (Discovering, Connecting, Monitoring) shared by the MainComponent of Mema.Mo and Mema.Re.
 *
 * Holds the discover and connecting panels, the TCP client connection to the selected Mema instance and the retry timer.
 * What happens on a received message, a lost connection or a rejected hello is app specific and stays in the derived class.
 */
class MemaClientMainComponentBase : public juce::Component,
                                    public juce::Timer
{
public:
    /** @brief Connection/application phase driven by the TCP session lifecycle. */
    enum Status
    {
        Discovering,    ///< Searching for Mema instances via multicast.
        Connecting,     ///< TCP handshake in progress.
        Monitoring,     ///< Connected and actively exchanging data (receiving for Mema.Mo, controlling for Mema.Re).
    };

    /** @brief Menu ids of the look-and-feel entries of the settings menu; the app specific settings option enums start with the same values. */
    enum SettingsLookAndFeelId
    {
        SettingsLookAndFeel_FollowHost = 1, ///< Match the host OS appearance.
        SettingsLookAndFeel_Dark,           ///< Force dark look-and-feel.
        SettingsLookAndFeel_Light,          ///< Force light look-and-feel.
    };

    //==============================================================================
    /** @brief Periodic callback used to retry TCP connections and poll network status. */
    void timerCallback() override;

    /** @brief Handles keyboard shortcuts (toggle fullscreen mode). */
    bool keyPressed(const juce::KeyPress& key) override;

    /** @brief Returns true if the application window currently is in fullscreen/kiosk mode (always true on iOS, Android and Linux). */
    bool isFullscreenEnabled();

    //==============================================================================
    std::function<void(int, bool)> onPaletteStyleChange;    ///< Called when the user changes the look-and-feel or the accent colour.
    std::function<void(bool)> onSetFullscreenWindow;        ///< Called to request a fullscreen/windowed transition from the application shell.

protected:
    /**
     * @class InterprocessConnectionImpl
     * @brief Thin wrapper around juce::InterprocessConnection that forwards events via std::function callbacks.
     *
     * All three JUCE interprocess events are forwarded to lambdas set by the main component so that
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
        void messageReceived(const juce::MemoryBlock& message) override { if (onMessageReceived) onMessageReceived(message); };

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

        std::function<void()>                       onConnectionMade;       ///< Fired on the JUCE IPC thread when the TCP session is established.
        std::function<void()>                       onConnectionLost;       ///< Fired on the JUCE IPC thread when the TCP session is dropped.
        std::function<void(const juce::MemoryBlock&)> onMessageReceived;    ///< Fired for every complete framed message received from Mema.

    private:
        juce::String m_hostName;    ///< Hostname or IP address of the last connection target.
        int m_portNumber = 0;       ///< Port of the last connection target.
    };

    //==============================================================================
    void handleSettingsLookAndFeelMenuResult(int selectedId);
    void handleSettingsFullscreenModeToggleResult();
    void toggleFullscreenMode();

    /** @brief Shows the modal dialog to set or clear the pairing token and persists it without a watcher update. */
    void showPairingTokenDialog();

    void setStatus(const Status& s);
    const Status getStatus();

    /** @brief Starts a connection attempt to m_selectedService and the retry timer. */
    void connectToMema();

    //==============================================================================
    JUCEAppBasics::SessionMasterAwareService        m_selectedService;          ///< Multicast service descriptor of the Mema instance chosen by the user.
    std::unique_ptr<InterprocessConnectionImpl>     m_networkConnection;        ///< Active TCP client socket (null while Discovering).

    std::unique_ptr<MemaClientDiscoverComponent>    m_discoverComponent;        ///< Service-discovery panel (Discovering phase).
    std::unique_ptr<MemaClientConnectingComponent>  m_connectingComponent;      ///< Connection-progress panel (Connecting phase).

    std::map<int, std::pair<std::string, int>>      m_settingsItems;            ///< Mapping from menu item ID to {label, app specific settings option}.
    int                                             m_settingsHostLookAndFeelId = -1; ///< Menu ID assigned to the "Follow host" look-and-feel entry.

    juce::String                                    m_pairingToken;             ///< Token sent to Mema in this client's HelloMessage (empty = none configured).

    bool                                            m_connectionRejected = false; ///< Set when Mema answered with HelloReject; the following connection loss returns to discover instead of reconnecting.
    Status                                          m_currentStatus = Status::Discovering; ///< Current state-machine phase.
};

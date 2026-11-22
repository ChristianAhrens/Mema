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
#include "MemaClientCommon/MemaClientMainComponentBase.h"

#include <ServiceTopologyManager.h>


class MemaReComponent;
class MemaClientDiscoverComponent;
class MemaClientConnectingComponent;
namespace JUCEAppBasics { class AboutComponent; }

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
class MainComponent :   public MemaClientMainComponentBase,
                        public MemaReAppConfiguration::Dumper,
                        public MemaReAppConfiguration::Watcher
{
public:
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
        PairingToken,                               ///< Set/clear the pairing token sent to Mema in this client's Hello.
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

    //==============================================================================
    /** @brief Serialises the current configuration to the XML file on disk. */
    void performConfigurationDump() override;
    /** @brief Reacts to external configuration changes. */
    void onConfigUpdated() override;

    //==============================================================================

private:
    //==============================================================================
    void handleSettingsMenuResult(int selectedId);
    void handleSettingsControlFormatMenuResult(int selectedId);
    void handleSettingsControlColourMenuResult(int selectedId);
    void handleSettingsControlsSizeMenuResult(int selectedId);
    void showExternalControlSettings();

    void setControlColour(const juce::Colour& meteringColour);
    void applyControlColour();

    void setControlsSize(const Mema::MemaClientControlComponentBase::ControlsSize& controlsSize);

    void returnToDiscover();

    //==============================================================================

    std::unique_ptr<MemaReComponent>                m_remoteComponent;          ///< Active remote-control panel (Controlling phase).

    std::unique_ptr<juce::DrawableButton>           m_settingsButton;           ///< Gear icon that opens the settings popup menu.

    std::unique_ptr<juce::DrawableButton>           m_disconnectButton;         ///< Button to drop the current TCP connection and return to Discovering.

    std::unique_ptr<juce::DrawableButton>           m_aboutButton;              ///< Button that shows the about popup.
    std::unique_ptr<JUCEAppBasics::AboutComponent>                 m_aboutComponent;           ///< The about popup component.

    std::unique_ptr<juce::AlertWindow>              m_messageBox;               ///< General-purpose alert dialog (e.g. ADM-OSC settings errors).

    juce::Colour                                    m_controlColour = juce::Colours::forestgreen; ///< Active accent colour for control elements.

    std::unique_ptr<MemaReAppConfiguration>         m_config;                   ///< XML configuration manager.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


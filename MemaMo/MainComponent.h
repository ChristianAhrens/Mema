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

#include "MemaMoAppConfiguration.h"

#include "MemaClientCommon/MemaClientMainComponentBase.h"

#include <ServiceTopologyManager.h>


class MemaMoComponent;
class MemaClientDiscoverComponent;
class MemaClientConnectingComponent;
namespace JUCEAppBasics { class AboutComponent; }

/**
 * @class MainComponent
 * @brief Top-level application component for **Mema.Mo** (MenubarMatrixMonitor).
 *
 * MainComponent drives the three-phase connection state machine of Mema.Mo and owns
 * all child components that are active in each phase:
 *
 * | Phase | Active child | Description |
 * |-------|--------------|-------------|
 * | Discovering | `MemaClientDiscoverComponent` | Listens for multicast service announcements from Mema instances on the local network. |
 * | Connecting  | `MemaClientConnectingComponent` | Establishes the TCP connection to the selected Mema server. |
 * | Monitoring  | `MemaMoComponent` | Receives streaming audio buffers and renders the chosen visualisation. |
 *
 * It also owns the TCP socket (`InterprocessConnectionImpl`), exposes a settings menu
 * for choosing look-and-feel, output visualisation type, and metering colour, and
 * persists all choices via `MemaMoAppConfiguration`.
 *
 * ## Role in the Mema tool suite
 * Mema.Mo is the **read-only monitoring companion** to the Mema audio-matrix server.
 * - Mema streams audio output buffers continuously over TCP (port 55668).
 * - Mema.Mo receives these buffers, feeds them into a local `ProcessorDataAnalyzer`,
 *   and renders one of four selectable visualisations.
 * - No control messages are ever sent from Mema.Mo to Mema (use Mema.Re for control).
 *
 * @see MemaMoComponent   — the active monitoring panel.
 * @see MemaMoAppConfiguration — XML persistence of settings.
 */
class MainComponent :   public MemaClientMainComponentBase,
                        public MemaMoAppConfiguration::Dumper,
                        public MemaMoAppConfiguration::Watcher
{
public:
    /** @brief Identifiers for all user-configurable settings exposed via the settings popup menu. */
    enum MemaMoSettingsOption
    {
        LookAndFeel_First = 1,
        LookAndFeel_FollowHost = LookAndFeel_First, ///< Match the host OS appearance.
        LookAndFeel_Dark,                           ///< Force dark look-and-feel.
        LookAndFeel_Light,                          ///< Force light look-and-feel.
        LookAndFeel_Last = LookAndFeel_Light,
        OutputVisuType_First,
        OutputVisuType_Meterbridge = OutputVisuType_First, ///< Level-bar meterbridge (default).
        OutputVisuType_LRS,                         ///< 3-channel Left/Right/Surround 2-D field.
        OutputVisuType_LCRS,                        ///< 4-channel LCRS 2-D field.
        OutputVisuType_5point0,                     ///< 5.0 surround 2-D field.
        OutputVisuType_5point1,                     ///< 5.1 surround 2-D field.
        OutputVisuType_5point1point2,               ///< 5.1.2 with two height channels.
        OutputVisuType_7point0,                     ///< 7.0 surround 2-D field.
        OutputVisuType_7point1,                     ///< 7.1 surround 2-D field.
        OutputVisuType_7point1point4,               ///< 7.1.4 Dolby Atmos layout.
        OutputVisuType_9point1point6,               ///< 9.1.6 ATMOS full-3D layout.
        OutputVisuType_Quadrophonic,                ///< Classic 4-channel quadrophonic 2-D field.
        OutputVisuType_Waveform,                    ///< Scrolling waveform plot.
        OutputVisuType_Spectrum,                    ///< FFT frequency-spectrum display.
        OutputVisuType_Last = OutputVisuType_Spectrum,
        MeteringColour_First,
        MeteringColour_Green = MeteringColour_First, ///< Green metering bars (default).
        MeteringColour_Red,                         ///< Red metering bars.
        MeteringColour_Blue,                        ///< Blue metering bars.
        MeteringColour_Pink,                        ///< Pink metering bars.
        MeteringColour_Laser,                       ///< High-visibility laser-green metering bars.
        MeteringColour_Last = MeteringColour_Laser,
        FullscreenWindowMode,                       ///< Toggle between popup and fullscreen window mode.
        PairingToken,                                ///< Set/clear the pairing token sent to Mema in this client's Hello.
    };

public:
    MainComponent();
    ~MainComponent() override;

    /** @brief Applies a settings menu selection, updating look-and-feel, visualisation type, or colour. */
    void applySettingsOption(const MemaMoSettingsOption& option);

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
    void handleSettingsOutputVisuTypeMenuResult(int selectedId);
    void handleSettingsMeteringColourMenuResult(int selectedId);

    void setMeteringColour(const juce::Colour& meteringColour);
    void applyMeteringColour();

    std::optional<int>  getNumVisibleChannels();

    void returnToDiscover();

    //==============================================================================

    std::unique_ptr<MemaMoComponent>                m_monitorComponent;         ///< Active monitoring panel (Monitoring phase).

    std::unique_ptr<juce::DrawableButton>           m_settingsButton;           ///< Gear icon that opens the settings popup menu.

    std::unique_ptr<juce::DrawableButton>           m_disconnectButton;         ///< Button to drop the current TCP connection and return to Discovering.

    std::unique_ptr<juce::DrawableButton>           m_aboutButton;              ///< Button that shows the about popup.
    std::unique_ptr<JUCEAppBasics::AboutComponent>                 m_aboutComponent;           ///< The about popup component.

    juce::Colour                                    m_meteringColour = juce::Colours::forestgreen; ///< Active metering bar colour.

    std::unique_ptr<MemaMoAppConfiguration>         m_config;                   ///< XML configuration manager.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


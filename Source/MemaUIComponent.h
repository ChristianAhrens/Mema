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

#include "MemaAppConfiguration.h"

#include <CustomLookAndFeel.h>
#include <ServiceTopologyManager.h>

class LoadBar;
class NetworkHealthBar;
class EmptySpace;
class AboutComponent;

namespace Mema
{

/**
 * @class MemaUIComponent
 * @brief Main UI shell for the Mema application — composes the processor editor, toolbar, load bars, and settings menu.
 *
 * @details MemaUIComponent is the outermost visible component shown to the user, both in
 * menubar/tray popup mode and in standalone window mode.  It:
 *
 * - Hosts the `MemaProcessorEditor` (set via `setEditorComponent()`) in its central area.
 * - Provides a toolbar row with buttons: standalone-window toggle, settings gear, audio setup,
 *   about, and power/quit.
 * - Displays a CPU load bar (`m_sysLoadBar`) and a network health bar (`m_netHealthBar`) that
 *   are updated from the `Mema` singleton's timer callback via `updateCpuUsageBar()` and
 *   `updateNetworkUsage()`.
 * - Exposes a settings popup menu (gear icon) for look-and-feel, metering colour, load/save config.
 * - Persists its own state (standalone mode, palette, metering colour) via `XmlConfigurableElement`
 *   within the main `MemaAppConfiguration` XML under `<UICONFIG>`.
 * - Reacts to OS dark-mode changes via `juce::DarkModeSettingListener` when "Automatic" palette is selected.
 *
 * ## Callback wiring
 * The `Mema` singleton connects lambdas to all `std::function` members during construction,
 * allowing `MemaUIComponent` to trigger application-level actions (window resize, audio setup,
 * config load/save) without a direct back-reference to `Mema`.
 *
 * @see Mema — the singleton that owns this component and wires up its callbacks.
 * @see MemaProcessorEditor — the embedded audio visualisation and routing control component.
 */
class MemaUIComponent : public juce::Component,
                        public juce::DarkModeSettingListener,
                        public JUCEAppBasics::AppConfigurationBase::XmlConfigurableElement
{
public:
    /**
     * @brief Identifiers for all user-configurable settings exposed via the settings popup menu.
     * @details Values are grouped into three ranges: look-and-feel (1–3), metering colour (4–8),
     *          and action items (load/save config).  The settings menu is built dynamically from
     *          `m_settingsItems` which maps each value to a localised display string and a submenu ID.
     */
    enum MemaSettingsOption
    {
        LookAndFeel_First = 1,
        LookAndFeel_Automatic = LookAndFeel_First, ///< Match the host OS appearance.
        LookAndFeel_Dark,                          ///< Force dark look-and-feel.
        LookAndFeel_Light,                         ///< Force light look-and-feel.
        LookAndFeel_Last = LookAndFeel_Light,
        MeteringColour_First,
        MeteringColour_Green = MeteringColour_First, ///< Green metering bars (default).
        MeteringColour_Red,                          ///< Red metering bars.
        MeteringColour_Blue,                         ///< Blue metering bars.
        MeteringColour_Pink,                         ///< Pink metering bars.
        MeteringColour_Laser,                        ///< High-visibility laser-green metering bars.
        MeteringColour_Last = MeteringColour_Laser,
        LoadConfig,  ///< Load configuration from file.
        SaveConfig   ///< Save configuration to file.
    };

public:
    /** @brief Constructs the UI component and wires up child components. */
    MemaUIComponent();
    ~MemaUIComponent() override;

    /**
     * @brief Switches between popup mode (embedded in the system tray/menubar popover) and standalone window mode.
     * @details In standalone mode the component fills a normal resizable OS window; in popup mode it is
     *          constrained to fit inside the system-provided popup panel.  The `onToggleStandaloneWindow`
     *          callback is fired so the `Mema` singleton can show or hide the dedicated window.
     * @param standalone `true` to show as a standalone window, `false` for popup/tray mode.
     */
    void setStandaloneWindow(bool standalone);
    /** @brief Returns `true` when the component is currently shown in standalone window mode. */
    bool isStandaloneWindow();

    /**
     * @brief Attaches the `MemaProcessorEditor` to be displayed in the central content area.
     * @details The component pointer is borrowed (not owned); the editor is owned by `MemaProcessor`.
     *          Calling this again with a different pointer replaces the previous editor.
     * @param editorComponent Pointer to the processor editor, or `nullptr` to clear it.
     */
    void setEditorComponent(juce::Component* editorComponent);

    /**
     * @brief Responds to a resize request from the embedded editor.
     * @details Called by `Mema::onEditorSizeChangeRequested` when the editor (or a loaded plugin)
     *          wants to be displayed at a specific size.  In standalone mode this resizes the window;
     *          in popup mode it adjusts the popup panel size if the host allows it.
     * @param requestedSize The new desired bounds for the editor area.
     */
    void handleEditorSizeChangeRequest(const juce::Rectangle<int>& requestedSize);

    /**
     * @brief Updates the CPU-load progress bar to reflect the current audio processing load.
     * @details Called from the `Mema` singleton's `juce::Timer` callback (~1 Hz) after querying
     *          `AudioDeviceManager::getCpuUsage()`.
     * @param loadPercent CPU load as a percentage in [0, 100].
     */
    void updateCpuUsageBar(int loadPercent);
    /**
     * @brief Updates the network-health bar with per-connection traffic metrics.
     * @details Called from the `Mema` singleton's timer callback after querying
     *          `MemaProcessor::getNetworkHealth()`.  Each entry represents one connected TCP client
     *          (Mema.Mo or Mema.Re).
     * @param netLoads Map of {connectionId → {bytes/s, isConnected}} for each active TCP client.
     */
    void updateNetworkUsage(const std::map<int, std::pair<double, bool>>& netLoads);
    /**
     * @brief Updates the service-topology display (if any) with freshly discovered Mema instances.
     * @details Called when `ServiceTopologyManager` detects a change in the multicast announcement list.
     *          Currently used to keep internal state; future UI may display a list of discoverable peers.
     * @param serviceDiscoveryTopologyString The full current service topology snapshot.
     */
    void updateSessionServiceTopology(const JUCEAppBasics::SessionServiceTopology& serviceDiscoveryTopologyString);

    //==============================================================================
    /**
     * @brief Responds to an OS dark-mode toggle.
     * @details Only acts when `m_followLocalStyle` is `true` (i.e. "Automatic" look-and-feel is selected).
     *          Reads the current OS setting and calls `applyPaletteStyle()` with the matching palette.
     */
    void darkModeSettingChanged() override;

    //========================================================================*
    /** @brief Paints the component background using the current look-and-feel background colour. */
    void paint(Graphics&) override;
    /**
     * @brief Lays out all child components.
     * @details Arranges the toolbar buttons and load bars in a fixed-height top strip, and scales
     *          the embedded editor to fill the remaining area below it.
     */
    void resized() override;

    //========================================================================*
    /** @brief Propagates a look-and-feel change to all child buttons and bars. */
    void lookAndFeelChanged() override;

    //==========================================================================
    /**
     * @brief Serialises the UI state (standalone mode, palette style, metering colour) to XML.
     * @return A new `<UICONFIG>` XmlElement for inclusion in the main application configuration.
     */
    std::unique_ptr<juce::XmlElement> createStateXml() override;
    /**
     * @brief Restores UI state from a `<UICONFIG>` XmlElement.
     * @param stateXml Pointer to the previously serialised XML element.
     * @return `true` on success; `false` if the element is null or malformed.
     */
    bool setStateXml(juce::XmlElement* stateXml) override;

    //========================================================================*
    std::function<void(bool)>   onToggleStandaloneWindow;  ///< Called when the user toggles standalone/popup mode.
    std::function<void()>       onAudioSetupMenuClicked;   ///< Called when the audio-setup button is clicked.
    std::function<void()>       onLookAndFeelChanged;      ///< Called after a look-and-feel change is applied.
    std::function<void()>       onDeleted;                 ///< Called when this component is destroyed.
    std::function<void()>       onSettingsChanged;         ///< Called when any setting changes.
    std::function<void(const JUCEAppBasics::CustomLookAndFeel::PaletteStyle&)>  onPaletteStyleChange; ///< Called when palette style changes.
    std::function<void()>       onLoadConfig;              ///< Called when the user selects Load Config.
    std::function<void()>       onSaveConfig;              ///< Called when the user selects Save Config.

private:
    //========================================================================*
    /** @brief Dispatches a top-level settings menu selection to the appropriate sub-handler. @param selectedId The menu item ID returned by the popup menu. */
    void handleSettingsMenuResult(int selectedId);
    /** @brief Applies a look-and-feel change selected from the settings submenu. @param selectedId The menu item ID corresponding to a `MemaSettingsOption` look-and-feel value. */
    void handleSettingsLookAndFeelMenuResult(int selectedId);
    /** @brief Applies a metering colour change selected from the settings submenu. @param selectedId The menu item ID corresponding to a `MemaSettingsOption` metering colour value. */
    void handleSettingsMeteringColourMenuResult(int selectedId);

    //========================================================================*
    /** @brief Stores a new metering colour and triggers `applyMeteringColour()`. @param meteringColour The new colour to use for metering bars. */
    void setMeteringColour(const juce::Colour& meteringColour);
    /** @brief Pushes the stored `m_meteringColour` to the processor editor's metering components. */
    void applyMeteringColour();

    /** @brief Applies a palette style to the look-and-feel and fires `onPaletteStyleChange`. @param paletteStyle The palette to apply (dark, light, or follow-host). */
    void applyPaletteStyle(const JUCEAppBasics::CustomLookAndFeel::PaletteStyle& paletteStyle);

    //========================================================================*
    std::unique_ptr<juce::DrawableButton>       m_toggleStandaloneWindowButton; ///< Button that switches between popup and standalone window mode.
    std::unique_ptr<juce::DrawableButton>       m_appSettingsButton; ///< Gear button that opens the settings popup menu.
    std::unique_ptr<juce::DrawableButton>       m_audioSetupButton; ///< Button that opens the JUCE audio device setup panel.
    std::unique_ptr<juce::DrawableButton>       m_aboutButton; ///< Button that shows the about component popup.
    std::unique_ptr<juce::DrawableButton>       m_powerButton; ///< Button that quits/closes the application.
    std::unique_ptr<EmptySpace>                 m_emptySpace; ///< Spacer component used in the toolbar layout.
    std::unique_ptr<LoadBar>                    m_sysLoadBar; ///< Horizontal progress bar showing CPU audio processing load (0–100 %).
    std::unique_ptr<LoadBar>                    m_netHealthBar; ///< Horizontal bar showing aggregate network traffic to/from connected clients.

    juce::Component* m_editorComponent = nullptr; ///< Borrowed pointer to the MemaProcessorEditor; not owned by this component.

    std::unique_ptr<AboutComponent>             m_aboutComponent; ///< The about popup component shown when m_aboutButton is clicked.

    std::map<int, std::pair<std::string, int>>  m_settingsItems; ///< Mapping from settings menu item ID to {display label, MemaSettingsOption value}.

    bool m_followLocalStyle = true; ///< True when "Automatic" look-and-feel is active and the component should track OS dark-mode changes.

    bool m_isStandaloneWindow = false; ///< True when the component is displayed in a standalone OS window rather than a tray popup.

    juce::Colour m_meteringColour = juce::Colours::forestgreen; ///< Currently active metering bar colour.

    static constexpr int sc_buttonSize = 26; ///< Fixed width and height of all toolbar icon buttons, in pixels.
    static constexpr int sc_loadNetWidth = 70; ///< Fixed width of the CPU and network load bars, in pixels.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaUIComponent)
};

};
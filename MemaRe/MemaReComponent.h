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

#include "MemaClientCommon/FaderbankControlComponent.h"
#include "MemaClientCommon/PanningControlComponent.h"
#include "MemaClientCommon/PluginControlComponent.h"

#include <MemaProcessor/MemaPluginParameterInfo.h>

/**
 * @class MemaReComponent
 * @brief Central remote-control panel of the Mema.Re application.
 *
 * MemaReComponent is the active-state UI of the Mema.Re tool — the remote-control
 * companion in the **Mema tool suite**.  When a TCP connection to a Mema server has
 * been established, this component owns three pluggable control modes and switches
 * between them based on the settings chosen in `MainComponent`:
 *
 * | Mode | Component | Description |
 * |------|-----------|-------------|
 * | Faderbank | `FaderbankControlComponent` | Slider/mute matrix for direct input × output crosspoint control. |
 * | 2-D panning | `PanningControlComponent` + `TwoDFieldMultisliderComponent` | Interactive spatial field for LRS up to 9.1.6 ATMOS layouts. |
 * | Plugin parameters | `PluginControlComponent` | Per-parameter controls (slider/combobox/toggle) rendered dynamically from `MemaPluginParameterInfo`. |
 *
 * Inbound TCP messages (`ControlParametersMessage`, `PluginParameterInfosMessage`) are
 * dispatched via `handleMessage()` and used to keep the control state in sync with Mema.
 * User interactions produce updated `ControlParametersMessage` / `PluginParameterValueMessage`
 * payloads that are sent back to Mema through the `onMessageReadyToSend` callback.
 *
 * An `ADMOSController` instance (owned by `PanningControlComponent`) can additionally
 * receive ADM-OSC UDP packets from an external spatial-audio controller and forward
 * x/y/z/width/mute updates directly into the 2-D panning view.
 *
 * @see MainComponent      — owns this component and drives the Discovering/Connecting/Controlling state machine.
 * @see Mema::FaderbankControlComponent — fader/mute crosspoint control.
 * @see Mema::PanningControlComponent   — 2-D spatial panning control with optional ADM-OSC input.
 * @see Mema::PluginControlComponent    — dynamic plugin-parameter control.
 */
class MemaReComponent :   public juce::Component, juce::MessageListener
{
public:
    /** @brief Lifecycle state of the remote-control panel driven by the network connection status. */
    enum RunningStatus
    {
        Inactive,   ///< No TCP connection; component renders a placeholder.
        Standby,    ///< Connection established but no state snapshot received yet.
        Active      ///< Actively receiving and sending control data to/from Mema.
    };

public:
    MemaReComponent();
    ~MemaReComponent() override;

    /** @brief Switches to the faderbank (crosspoint slider/mute matrix) control mode. */
    void setFaderbankCtrlActive();
    /** @brief Switches to the 2-D spatial panning control for the given speaker layout. */
    void setOutputPanningCtrlActive(const juce::AudioChannelSet& channelConfiguration);
    /** @brief Switches to the plugin-parameter control mode. */
    void setPluginCtrlActive();
    /** @brief Resets all control components to their default/empty state. */
    void resetCtrl();

    /** @brief Propagates a control-element size change (S/M/L) to the faderbank component. */
    void setControlsSize(const Mema::FaderbankControlComponent::ControlsSize& ctrlsSize);
    /** @brief Returns the current control-element size setting. */
    const Mema::FaderbankControlComponent::ControlsSize getControlsSize();

    /** @brief Configures the ADM-OSC listener port and the remote-controller address used by the panning component. */
    void setExternalAdmOscSettings(const int ADMOSCport, const juce::IPAddress& ADMOSCremoteIP, const int ADMOSCremotePort);
    /** @brief Returns the current ADM-OSC settings as {listenPort, remoteIP, remotePort}. */
    std::tuple<int, juce::IPAddress, int> getExternalAdmOscSettings();

    //==============================================================================
    /** @brief Lays out the active control component to fill the available area. */
    void resized() override;
    /** @brief Paints the background when no control mode is active. */
    void paint(juce::Graphics& g) override;

    //==============================================================================
    /** @brief Dispatches inbound network messages to the appropriate control component. */
    void handleMessage(const Message& message) override;

    //==============================================================================
    std::function<void()>                           onExitClick;            ///< Invoked when the user triggers a disconnection.
    std::function<void(const juce::MemoryBlock&)>   onMessageReadyToSend;   ///< Invoked with a serialised message whenever a control value changes that must be sent to Mema.

private:
    //==============================================================================
    std::unique_ptr<Mema::FaderbankControlComponent>    m_faderbankCtrlComponent;   ///< Faderbank input×output crosspoint control.
    std::unique_ptr<Mema::PanningControlComponent>      m_panningCtrlComponent;     ///< 2-D spatial panning control (with embedded ADMOSController).
    std::unique_ptr<Mema::PluginControlComponent>       m_pluginCtrlComponent;      ///< Dynamic plugin-parameter control.

    //==============================================================================
    RunningStatus m_runningStatus = RunningStatus::Inactive;    ///< Current lifecycle state.
    static constexpr int sc_connectionTimeout = 5000;           ///< Milliseconds before a stalled connection attempt is treated as failed.

    std::pair<int, int>                                     m_currentIOCount = { 0, 0 };    ///< Current {input, output} channel count received from Mema.
    std::map<std::uint16_t, bool>                           m_inputMuteStates = {};          ///< Per-input mute state mirror (channel index → muted).
    std::map<std::uint16_t, bool>                           m_outputMuteStates = {};         ///< Per-output mute state mirror.
    std::map<std::uint16_t, std::map<std::uint16_t, bool>>  m_crosspointStates = {};         ///< Crosspoint enable state mirror (input → output → enabled).
    std::map<std::uint16_t, std::map<std::uint16_t, float>> m_crosspointValues = {};         ///< Crosspoint gain value mirror (input → output → linear gain).

    std::tuple<int, juce::IPAddress, int>   m_externalAdmOscSettings = { 4001, juce::IPAddress::local(), 4002 }; ///< ADM-OSC {listenPort, remoteIP, remotePort}.

    float m_ioRatio = 0.5f; ///< Vertical split ratio between input and output control areas (faderbank mode).

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaReComponent)
};


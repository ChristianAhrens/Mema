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

namespace Mema
{
    class ProcessorDataAnalyzer;
    class MeterbridgeComponent;
    class TwoDFieldOutputComponent;
    class WaveformAudioComponent;
    class SpectrumAudioComponent;
}

/**
 * @class MemaMoComponent
 * @brief Central monitoring panel of the Mema.Mo application.
 *
 * MemaMoComponent is the active-state UI of the Mema.Mo tool — the monitoring
 * companion in the **Mema tool suite**.  When a TCP connection to a Mema server has
 * been established, this component owns a pair of `ProcessorDataAnalyzer` instances
 * (one for input audio, one for output audio) that process the streamed
 * `AudioOutputBufferMessage` / `AudioInputBufferMessage` payloads received over the
 * network and broadcast level/spectrum data to the pluggable visualisation components.
 *
 * ## Visualisation modes (mutually exclusive, switched from MainComponent settings):
 * - **Meterbridge** — horizontal or vertical peak/RMS/hold level bars for every channel.
 * - **2-D spatial field** — positional level indicators for speaker layouts from LRS up
 *   to 9.1.6 ATMOS, rendered by `TwoDFieldOutputComponent`.
 * - **Waveform** — scrolling audio waveform plots per channel.
 * - **Spectrum** — frequency-domain FFT magnitude display per channel.
 *
 * The component communicates back to `MainComponent` via the `onExitClick` callback
 * when the user requests a disconnection.
 *
 * @see MainComponent  — owns this component and drives the Discovering/Connecting/Monitoring state machine.
 * @see Mema::ProcessorDataAnalyzer  — performs peak/RMS/hold metering and FFT analysis on received buffers.
 */
class MemaMoComponent :   public juce::Component, juce::MessageListener
{
public:
    /** @brief Lifecycle state of the monitor panel driven by the network connection status. */
    enum RunningStatus
    {
        Inactive,   ///< No TCP connection; component renders a placeholder.
        Standby,    ///< Connection established but no audio data received yet.
        Active      ///< Actively receiving and visualising audio data from Mema.
    };

public:
    MemaMoComponent();
    ~MemaMoComponent() override;

    /** @brief Switches to the meterbridge (level-bar) visualisation mode. */
    void setOutputMeteringVisuActive();
    /** @brief Switches to the 2-D spatial field visualisation for the given speaker layout. */
    void setOutputFieldVisuActive(const juce::AudioChannelSet& channelConfiguration);
    /** @brief Switches to the scrolling waveform visualisation mode. */
    void setWaveformVisuActive();
    /** @brief Switches to the FFT spectrum visualisation mode. */
    void setSpectrumVisuActive();

    /** @brief Returns the number of channels currently shown, or empty if no active visualiser. */
    std::optional<std::uint16_t> getNumVisibleChannels();
    /** @brief Propagates a channel-count change to the active visualisation component. */
    void setNumVisibleChannels(std::uint16_t count);

    //==============================================================================
    /** @brief Lays out visualisation components to fill the available area. */
    void resized() override;
    /** @brief Paints the background when no visualisation is active. */
    void paint(juce::Graphics& g) override;

    //==============================================================================
    /** @brief Dispatches inbound network messages to the appropriate analyzer or state handler. */
    void handleMessage(const Message& message) override;

    //==============================================================================
    std::function<void()>   onExitClick;    ///< Invoked when the user triggers a disconnection.

private:
    //==============================================================================
    std::unique_ptr<Mema::ProcessorDataAnalyzer>  m_inputDataAnalyzer;     ///< Analyzes streamed input audio buffers (peak/RMS/FFT).
    std::unique_ptr<Mema::ProcessorDataAnalyzer>  m_outputDataAnalyzer;    ///< Analyzes streamed output audio buffers (peak/RMS/FFT).

    std::unique_ptr<Mema::MeterbridgeComponent>     m_inputMeteringComponent;   ///< Input level-bar meterbridge (shown alongside output meters).
    std::unique_ptr<Mema::MeterbridgeComponent>     m_outputMeteringComponent;  ///< Output level-bar meterbridge.
    std::unique_ptr<Mema::TwoDFieldOutputComponent> m_outputFieldComponent;     ///< 2-D spatial output level display.
    std::unique_ptr<Mema::WaveformAudioComponent>   m_waveformComponent;        ///< Scrolling waveform plot.
    std::unique_ptr<Mema::SpectrumAudioComponent>   m_spectrumComponent;        ///< FFT spectrum display.

    //==============================================================================
    RunningStatus m_runningStatus = RunningStatus::Inactive;    ///< Current lifecycle state.
    static constexpr int sc_connectionTimeout = 5000;           ///< Milliseconds before a stalled connection attempt is treated as failed.

    std::pair<int, int> m_currentIOCount = { 0, 0 };   ///< Current {input, output} channel count received from Mema.

    float m_ioRatio = 0.5f; ///< Vertical split ratio between input and output meter areas.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaMoComponent)
};


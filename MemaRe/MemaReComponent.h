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

class MemaReComponent :   public juce::Component, juce::MessageListener
{
public:
    enum RunningStatus
    {
        Inactive,
        Standby,
        Active
    };

public:
    MemaReComponent();
    ~MemaReComponent() override;

    void setOutputFaderbankCtrlActive();
    void setOutputPanningCtrlActive(const juce::AudioChannelSet& channelConfiguration);
    void resetCtrl();

    void setControlsSize(const Mema::FaderbankControlComponent::ControlsSize& ctrlsSize);
    const Mema::FaderbankControlComponent::ControlsSize getControlsSize();

    void setExternalAdmOscSettings(const int ADMOSCport, const juce::IPAddress& ADMOSCremoteIP, const int ADMOSCremotePort);
    std::tuple<int, juce::IPAddress, int> getExternalAdmOscSettings();

    //==============================================================================
    void resized() override;
    void paint(juce::Graphics& g) override;

    //==============================================================================
    void handleMessage(const Message& message) override;

    //==============================================================================
    std::function<void()>                           onExitClick;
    std::function<void(const juce::MemoryBlock&)>   onMessageReadyToSend;

private:
    //==============================================================================
    std::unique_ptr<Mema::FaderbankControlComponent>    m_faderbankCtrlComponent;
    std::unique_ptr<Mema::PanningControlComponent>      m_panningCtrlComponent;

    //==============================================================================
    RunningStatus m_runningStatus = RunningStatus::Inactive;
    static constexpr int sc_connectionTimeout = 5000; // 5s running before attempt is considered failed

    std::pair<int, int>                                     m_currentIOCount = { 0, 0 };
    std::map<std::uint16_t, bool>                           m_inputMuteStates = {};
    std::map<std::uint16_t, bool>                           m_outputMuteStates = {};
    std::map<std::uint16_t, std::map<std::uint16_t, bool>>  m_crosspointStates = {};
    std::map<std::uint16_t, std::map<std::uint16_t, float>> m_crosspointValues = {};

    std::tuple<int, juce::IPAddress, int>   m_externalAdmOscSettings = { 4001, juce::IPAddress::local(), 4002 };

    float m_ioRatio = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaReComponent)
};


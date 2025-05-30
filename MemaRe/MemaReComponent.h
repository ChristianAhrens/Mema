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

namespace Mema
{
    //class ProcessorDataAnalyzer;
    //class MeterbridgeComponent;
    //class TwoDFieldOutputComponent;
}

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

    //==============================================================================
    void resized() override;
    void paint(juce::Graphics& g) override;

    //==============================================================================
    void handleMessage(const Message& message) override;

    //==============================================================================
    std::function<void()>   onExitClick;

private:
    //==============================================================================
    //std::unique_ptr<Mema::ProcessorDataAnalyzer>  m_inputDataAnalyzer;
    //std::unique_ptr<Mema::ProcessorDataAnalyzer>  m_outputDataAnalyzer;
    
    //std::unique_ptr<Mema::MeterbridgeComponent> m_inputMeteringComponent;
    //std::unique_ptr<Mema::MeterbridgeComponent> m_outputMeteringComponent;
    //std::unique_ptr<Mema::TwoDFieldOutputComponent> m_outputFieldComponent;

    //==============================================================================
    RunningStatus m_runningStatus = RunningStatus::Inactive;
    static constexpr int sc_connectionTimeout = 5000; // 5s running before attempt is considered failed

    std::pair<int, int>                                                         m_currentIOCount = { 0, 0 };
    std::map<std::uint16_t, bool>                                               m_inputMuteStates = {};
    std::map<std::uint16_t, bool>                                               m_outputMuteStates = {};
    std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>    m_crosspointStates = {};

    float m_ioRatio = 0.5f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaReComponent)
};


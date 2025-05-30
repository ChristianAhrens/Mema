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

class MemaClientControlComponentBase : public juce::Component
{
public:
    MemaClientControlComponentBase();
    virtual ~MemaClientControlComponentBase();

    //==============================================================================
    virtual void paint(Graphics&) = 0;
    virtual void resized() = 0;

    //==============================================================================
    void setIOCount(const std::pair<int, int>& ioCount);
    const std::pair<int, int>& getIOCount();

    void setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates);
    const std::map<std::uint16_t, bool>& getInputMuteStates();

    void setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates);
    const std::map<std::uint16_t, bool>& getOutputMuteStates();

    void setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& crosspointStates);
    const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& getCrosspointStates();

    //==============================================================================
    std::function<void(const std::map<std::uint16_t, bool>&)>                                               onInputMutesChanged;
    std::function<void(const std::map<std::uint16_t, bool>&)>                                               onOutputMutesChanged;
    std::function<void(const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>&)>    onCrosspointStatesChanged;

protected:
    //==============================================================================
    const juce::String getClientControlParametersAsString();

private:
    //==============================================================================
    std::pair<int, int>                                                         m_ioCount = { 0, 0 };
    std::map<std::uint16_t, bool>                                               m_inputMuteStates = {};
    std::map<std::uint16_t, bool>                                               m_outputMuteStates = {};
    std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>    m_crosspointStates = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaClientControlComponentBase)
};


class FaderbankControlComponent : public MemaClientControlComponentBase
{
public:
    FaderbankControlComponent();
    virtual ~FaderbankControlComponent();

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==============================================================================

    //==============================================================================

private:
    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FaderbankControlComponent)
};


class PanningControlComponent : public MemaClientControlComponentBase
{
public:
    PanningControlComponent();
    virtual ~PanningControlComponent();

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;

    //==============================================================================
    void setChannelConfig(const juce::AudioChannelSet& channelConfiguration);
    const juce::AudioChannelSet& getChannelConfig();

    //==============================================================================

private:
    //==============================================================================
    juce::AudioChannelSet   m_channelConfiguration;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PanningControlComponent)
};


}; // namespace Mema

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
    enum ControlDirection
    {
        None = 0,
        Input,
        Output
    };
    enum ControlsSize
    {
        S = 35,
        M = 50,
        L = 65
    };

    static constexpr int s_gap = 3;
    static constexpr int s_scrollbarsize = 8;

public:
    MemaClientControlComponentBase();
    virtual ~MemaClientControlComponentBase();

    //==============================================================================
    virtual void paint(Graphics&) = 0;
    virtual void resized() = 0;

    //==============================================================================
    virtual void resetCtrl() = 0;

    //==============================================================================
    virtual void setControlsSize(const ControlsSize& ctrlsSize);
    const ControlsSize& getControlsSize();

    virtual void setIOCount(const std::pair<int, int>& ioCount);
    const std::pair<int, int>& getIOCount();

    virtual void setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates);
    const std::map<std::uint16_t, bool>& getInputMuteStates();

    virtual void setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates);
    const std::map<std::uint16_t, bool>& getOutputMuteStates();

    virtual void setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates);
    const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& getCrosspointStates();

    virtual void setCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues);
    const std::map<std::uint16_t, std::map<std::uint16_t, float>>& getCrosspointValues();

    virtual void addCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates);
    virtual void addCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues);

    //==============================================================================
    std::function<void(const std::map<std::uint16_t, bool>&)>                           onInputMutesChanged;
    std::function<void(const std::map<std::uint16_t, bool>&)>                           onOutputMutesChanged;
    std::function<void(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>&)>  onCrosspointStatesChanged;
    std::function<void(const std::map<std::uint16_t, std::map<std::uint16_t, float>>&)> onCrosspointValuesChanged;

    //==============================================================================
    const juce::String getClientControlParametersAsString();
    const juce::String getIOCountParametersAsString();
    const juce::String getInputMuteParametersAsString();
    const juce::String getOutputMuteParametersAsString();
    const juce::String getCrosspointParametersAsString();

protected:
    //==============================================================================
    ControlsSize    m_controlsSize = ControlsSize::S;

private:
    //==============================================================================
    std::pair<int, int>                                     m_ioCount = { 0, 0 };
    std::map<std::uint16_t, bool>                           m_inputMuteStates = {};
    std::map<std::uint16_t, bool>                           m_outputMuteStates = {};
    std::map<std::uint16_t, std::map<std::uint16_t, bool>>  m_crosspointStates = {};
    std::map<std::uint16_t, std::map<std::uint16_t, float>> m_crosspointValues = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaClientControlComponentBase)
};


}; // namespace Mema

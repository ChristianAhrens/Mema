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
    virtual void resetCtrl() = 0;

    //==============================================================================
    virtual void setIOCount(const std::pair<int, int>& ioCount);
    const std::pair<int, int>& getIOCount();

    virtual void setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates);
    const std::map<std::uint16_t, bool>& getInputMuteStates();

    virtual void setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates);
    const std::map<std::uint16_t, bool>& getOutputMuteStates();

    virtual void setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& crosspointStates);
    const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& getCrosspointStates();

    //==============================================================================
    std::function<void(const std::map<std::uint16_t, bool>&)>                                               onInputMutesChanged;
    std::function<void(const std::map<std::uint16_t, bool>&)>                                               onOutputMutesChanged;
    std::function<void(const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>&)>    onCrosspointStatesChanged;

protected:
    //==============================================================================
    const juce::String getClientControlParametersAsString();
    const juce::String getIOCountParametersAsString();
    const juce::String getInputMuteParametersAsString();
    const juce::String getOutputMuteParametersAsString();
    const juce::String getCrosspointParametersAsString();

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
    enum ControlDirection
    {
        None = 0,
        Input,
        Output
    };

public:
    FaderbankControlComponent();
    virtual ~FaderbankControlComponent();

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;

    //==============================================================================
    void resetCtrl() override;

    //==============================================================================
    void setIOCount(const std::pair<int, int>& ioCount) override;
    void setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates) override;
    void setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates) override;
    void setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& crosspointStates) override;

    //==============================================================================
    void addCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& crosspointStates);

protected:
    //==============================================================================
    void selectIOChannel(const ControlDirection& direction, int channel);
    void rebuildControls();
    void updateCrosspointFaderValues();

private:
    //==============================================================================
    std::unique_ptr<juce::Grid>                     m_inputControlsGrid;
    std::vector<std::unique_ptr<juce::TextButton>>  m_inputSelectButtons;
    std::vector<std::unique_ptr<juce::TextButton>>  m_inputMuteButtons;

    std::unique_ptr<juce::Grid>                     m_outputControlsGrid;
    std::vector<std::unique_ptr<juce::TextButton>>  m_outputSelectButtons;
    std::vector<std::unique_ptr<juce::TextButton>>  m_outputMuteButtons;

    std::unique_ptr<juce::Grid>                 m_crosspointsControlsGrid;
    std::vector<std::unique_ptr<juce::Slider>>  m_crosspointGainSliders;
    std::unique_ptr<juce::Label>                m_crosspointsNoSelectionLabel;

    std::pair<ControlDirection, int>    m_currentIOChannel = { ControlDirection::None, 0 };

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
    void resetCtrl() override;

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

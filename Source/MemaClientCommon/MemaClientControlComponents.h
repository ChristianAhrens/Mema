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


/**
 * Fwd. Decls.
 */
namespace JUCEAppBasics
{
    class ToggleStateSlider;
}

namespace Mema
{

class MemaClientControlComponentBase : public juce::Component
{
public:
    static constexpr int gap = 3;
    static constexpr int scrollbarsize = 8;

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

    virtual void setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates);
    const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& getCrosspointStates();

    virtual void setCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues);
    const std::map<std::uint16_t, std::map<std::uint16_t, float>>& getCrosspointValues();

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

private:
    //==============================================================================
    std::pair<int, int>                                     m_ioCount = { 0, 0 };
    std::map<std::uint16_t, bool>                           m_inputMuteStates = {};
    std::map<std::uint16_t, bool>                           m_outputMuteStates = {};
    std::map<std::uint16_t, std::map<std::uint16_t, bool>>  m_crosspointStates = {};
    std::map<std::uint16_t, std::map<std::uint16_t, float>> m_crosspointValues = {};

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
    enum ControlsSize
    {
        S = 35,
        M = 50,
        L = 65
    };

public:
    FaderbankControlComponent();
    virtual ~FaderbankControlComponent();

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;
    
    //==============================================================================
    void setControlsSize(const ControlsSize& ctrlsSize);

    //==============================================================================
    void resetCtrl() override;

    //==============================================================================
    void setIOCount(const std::pair<int, int>& ioCount) override;
    void setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates) override;
    void setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates) override;
    void setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates) override;
    void setCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues) override;

    //==============================================================================
    void addCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates);
    void addCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues);

protected:
    //==============================================================================
    void selectIOChannel(const ControlDirection& direction, int channel);
    void rebuildControls(bool force = false);
    void rebuildInputControls(bool force = false);
    void rebuildOutputControls(bool force = false);
    void rebuildCrosspointControls(bool force = false);
    void updateCrosspointFaderValues();

private:
    //==============================================================================
    std::unique_ptr<juce::Viewport>     m_horizontalScrollViewport;
    std::unique_ptr<juce::Component>    m_horizontalScrollContainerComponent;
    std::unique_ptr<juce::Viewport>     m_verticalScrollViewport;
    std::unique_ptr<juce::Component>    m_verticalScrollContainerComponent;

    std::unique_ptr<juce::Grid>                     m_inputControlsGrid;
    std::vector<std::unique_ptr<juce::TextButton>>  m_inputSelectButtons;
    std::vector<std::unique_ptr<juce::TextButton>>  m_inputMuteButtons;

    std::unique_ptr<juce::Grid>                     m_outputControlsGrid;
    std::vector<std::unique_ptr<juce::TextButton>>  m_outputSelectButtons;
    std::vector<std::unique_ptr<juce::TextButton>>  m_outputMuteButtons;

    std::unique_ptr<juce::Grid>                                     m_crosspointsControlsGrid;
    std::vector<std::unique_ptr<JUCEAppBasics::ToggleStateSlider>>  m_crosspointGainSliders;
    std::unique_ptr<juce::Label>                                    m_crosspointsNoSelectionLabel;

    std::pair<ControlDirection, int>    m_currentIOChannel = { ControlDirection::None, 0 };
    
    ControlsSize m_controlsSize = ControlsSize::S;

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

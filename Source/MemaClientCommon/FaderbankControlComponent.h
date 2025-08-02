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

#include "MemaClientControlComponentBase.h"


/**
 * Fwd. Decls.
 */
namespace JUCEAppBasics
{
    class ToggleStateSlider;
}
namespace Mema
{
    class TwoDFieldMultisliderComponent;
}

namespace Mema
{


class FaderbankControlComponent : public MemaClientControlComponentBase
{
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
    void setControlsSize(const ControlsSize& ctrlsSize) override;

    //==============================================================================
    void setIOCount(const std::pair<int, int>& ioCount) override;
    void setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates) override;
    void setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates) override;
    void setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates) override;
    void setCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues) override;

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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FaderbankControlComponent)
};


}; // namespace Mema

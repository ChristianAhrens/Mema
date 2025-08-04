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

#include "AbstractAudioVisualizer.h"
#include "../MemaProcessor/MemaCommanders.h"


namespace Mema
{

/**
 * fwd. decls.
 */
class MeterbridgeComponent;

//==============================================================================
/*
*/
class InputControlComponent :   public AbstractAudioVisualizer,
                                public MemaInputCommander,
                                public juce::TextButton::Listener
{
public:
    InputControlComponent();
    ~InputControlComponent();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    //==============================================================================
    void processingDataChanged(AbstractProcessorData *data) override;

    //==============================================================================
    virtual void processChanges() override;

    //==============================================================================
    void setInputMute(std::uint16_t channel, bool muteState, int userId = -1) override;

    //==============================================================================
    void buttonClicked(juce::Button*) override;

    //==============================================================================
    juce::Rectangle<int> getRequiredSize();
    std::function<void()> onBoundsRequirementChange;

    //==============================================================================
    void setChannelCount(std::uint16_t channelCount) override;

private:
    //==============================================================================
    ProcessorLevelData                                      m_levelData;
    std::unique_ptr<MeterbridgeComponent>                   m_inputLevels;
    std::map<std::uint16_t, std::unique_ptr<TextButton>>    m_inputMutes;
    std::uint16_t                                           m_channelCount = 0;

    static constexpr int s_channelSize = 24;
    static constexpr double s_channelGap = 1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InputControlComponent)
};

}

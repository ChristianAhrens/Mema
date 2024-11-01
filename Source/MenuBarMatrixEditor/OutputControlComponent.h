/* Copyright (c) 2024, Christian Ahrens
 *
 * This file is part of MenuBarMatrix <https://github.com/ChristianAhrens/MenuBarMatrix>
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
#include "../MenuBarMatrixProcessor/MenuBarMatrixCommanders.h"


namespace MenuBarMatrix
{

/**
 * fwd. decls.
 */
class MeterbridgeComponent;

//==============================================================================
/*
*/
class OutputControlComponent :  public AbstractAudioVisualizer,
                                public MenuBarMatrixOutputCommander,
                                public juce::TextButton::Listener
{
public:
    OutputControlComponent();
    ~OutputControlComponent();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    //==============================================================================
    void processingDataChanged(AbstractProcessorData *data) override;

    //==============================================================================
    virtual void processChanges() override;

    //==============================================================================
    void setOutputMute(unsigned int channel, bool muteState) override;

    //==============================================================================
    void buttonClicked(juce::Button*) override;

    //==============================================================================
    juce::Rectangle<int> getRequiredSize();
    std::function<void()> onBoundsRequirementChange;

    //==============================================================================
    void setChannelCount(int channelCount) override;

private:
    //==============================================================================
    ProcessorLevelData                          m_levelData;

    std::unique_ptr<MeterbridgeComponent>       m_outputLevels;
    std::map<int, std::unique_ptr<TextButton>>  m_outputMutes;
    int m_channelCount = 0;

    static constexpr int s_channelSize = 24;
    static constexpr double s_channelGap = 1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputControlComponent)
};

}

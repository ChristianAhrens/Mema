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

#include "../MemaProcessor/MemaProcessor.h"

namespace Mema
{

/** @class AbstractAudioVisualizer
 *  @brief Base class for all audio-data visualisation components in the Mema processor editor.
 *
 * Implements juce::Timer and ProcessorDataAnalyzer::Listener to receive level/spectrum
 * notifications and trigger repaints at a configurable refresh rate.
 */
class AbstractAudioVisualizer : public juce::Component,
                                public ProcessorDataAnalyzer::Listener,
                                public juce::Timer
{
public:
    AbstractAudioVisualizer();
    virtual ~AbstractAudioVisualizer();
    
    //==============================================================================
    /** @brief Marks that new data is available and triggers a repaint on the next timer tick. */
    void notifyChanges();
    /** @brief Called on the message thread to update cached data before repainting. */
    virtual void processChanges();

    //==============================================================================
    /** @brief Paints the visualiser background. */
    void paint (Graphics&) override;
    /** @brief Lays out child components. */
    void resized() override;
    /** @brief Handles mouse-down events (e.g. right-click context menu). */
    void mouseDown(const MouseEvent& event) override;

    //==============================================================================
    /** @brief Sets the display refresh rate in Hz. */
    void setRefreshFrequency(int frequency);
    /** @brief Timer callback that calls processChanges() and triggers a repaint if data changed. */
    void timerCallback() override;

protected:
    //==============================================================================
    void setUsesValuesInDB(bool useValuesInDB);
    bool getUsesValuesInDB();

private:
    bool    m_changesPending{ false };
    bool    m_usesValuesInDB{ 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AbstractAudioVisualizer)
};

}

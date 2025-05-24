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


class MemaConnectingComponent :   public juce::Component
{
public:
    MemaConnectingComponent();
    ~MemaConnectingComponent() override;

    //==============================================================================
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    double                                  m_progress = -1.0;
    std::unique_ptr<juce::ProgressBar>      m_startupProgressIndicator;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemaConnectingComponent)
};


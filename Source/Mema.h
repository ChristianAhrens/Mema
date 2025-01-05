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

#include "AppConfiguration.h"


namespace Mema
{

/**
 * Fwd. decls
 */
class AudioSelectComponent;
class MemaEditor;
class MemaProcessor;
class MemaRemoteWrapper;

//==============================================================================
/*
 *
 */
class Mema   : public juce::Component,
                        public juce::Timer,
                        public AppConfiguration::Dumper,
                        public AppConfiguration::Watcher
{
public:
    Mema();
    ~Mema() override;

    //==========================================================================
    void timerCallback() override;

    //==========================================================================
    juce::Component* getUIComponent();
    juce::Component* getDeviceSetupComponent();

    //==========================================================================
    std::function<void(int)> onCpuUsageUpdate;
    std::function<void(std::map<int, std::pair<double, bool>>)> onNetworkUsageUpdate;
    std::function<void(juce::Rectangle<int>)> onSizeChangeRequested;

    //==========================================================================
    void performConfigurationDump() override;
    void onConfigUpdated() override;

    //==========================================================================
    void lookAndFeelChanged() override;

private:
    std::unique_ptr<MemaProcessor>          m_MemaProcessor;

    std::unique_ptr<MemaEditor>             m_audioVisuComponent;
    std::unique_ptr<AudioSelectComponent>   m_audioDeviceSelectComponent;

    std::unique_ptr<AppConfiguration>       m_config;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Mema)
};

};

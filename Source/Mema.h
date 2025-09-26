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

#include "MemaAppConfiguration.h"


namespace Mema
{

/**
 * Fwd. decls
 */
class AudioSelectComponent;
class MemaProcessorEditor;
class MemaProcessor;
class MemaRemoteWrapper;

//==============================================================================
/*
 *
 */
class Mema   :  public juce::Timer,
                public MemaAppConfiguration::Dumper,
                public MemaAppConfiguration::Watcher
{
public:
    Mema();
    ~Mema() override;

    //==========================================================================
    void timerCallback() override;

    //==========================================================================
    juce::Component* getMemaProcessorEditor();
    juce::Component* getDeviceSetupComponent();

    //==========================================================================
    std::function<void(int)> onCpuUsageUpdate;
    std::function<void(std::map<int, std::pair<double, bool>>)> onNetworkUsageUpdate;
    std::function<void(juce::Rectangle<int>)> onEditorSizeChangeRequested;
    
    void clearUICallbacks();

    //==========================================================================
    void performConfigurationDump() override;
    void onConfigUpdated() override;

    //==========================================================================
    void propagateLookAndFeelChanged();

    void setUIConfigState(const std::unique_ptr<juce::XmlElement>& uiConfigState);
    const std::unique_ptr<juce::XmlElement>& getUIConfigState();

    void triggerPromptLoadConfig();
    void triggerPromptSaveConfig();

    JUCE_DECLARE_SINGLETON(Mema, false)

private:
    std::unique_ptr<MemaProcessor>          m_MemaProcessor;
    std::unique_ptr<juce::XmlElement>       m_MemaUIConfigCache;

    std::unique_ptr<MemaProcessorEditor>    m_audioVisuComponent;
    std::unique_ptr<AudioSelectComponent>   m_audioDeviceSelectComponent;

    std::unique_ptr<MemaAppConfiguration>   m_config;

    std::unique_ptr<juce::FileChooser>      m_loadSavefileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mema)
};

};

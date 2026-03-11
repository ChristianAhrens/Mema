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

#include <ServiceTopologyManager.h>


namespace Mema
{

/**
 * Fwd. decls
 */
class AudioSelectComponent;
class MemaProcessorEditor;
class MemaProcessor;
class MemaRemoteWrapper;

/** @class Mema
 *  @brief Singleton application controller managing the audio processor, UI, and configuration.
 */
class Mema   :  public juce::Timer,
                public MemaAppConfiguration::Dumper,
                public MemaAppConfiguration::Watcher
{
public:
    Mema();
    ~Mema() override;

    //==========================================================================
    /** @brief Periodic timer callback used to poll CPU usage and trigger deferred dumps. */
    void timerCallback() override;

    //==========================================================================
    /** @brief Returns the main processor editor component. */
    juce::Component* getMemaProcessorEditor();
    /** @brief Returns the audio device setup component. */
    juce::Component* getDeviceSetupComponent();

    //==========================================================================
    std::function<void(int)> onCpuUsageUpdate;                                                          ///< Called when CPU load percentage changes.
    std::function<void(const std::map<int, std::pair<double, bool>>&)> onNetworkUsageUpdate;            ///< Called when network traffic metrics change.
    std::function<void(const JUCEAppBasics::SessionServiceTopology&)> onServiceDiscoveryTopologyUpdate; ///< Called when the multicast service topology changes.
    std::function<void(juce::Rectangle<int>)> onEditorSizeChangeRequested;                             ///< Called when the editor requests a resize.

    /** @brief Clears all UI callback functions. */
    void clearUICallbacks();

    //==========================================================================
    /** @brief Serializes the current configuration to file. */
    void performConfigurationDump() override;
    /** @brief Reacts to configuration changes and updates internal state. */
    void onConfigUpdated() override;

    //==========================================================================
    /** @brief Propagates a look-and-feel change to all owned components. */
    void propagateLookAndFeelChanged();

    /** @brief Stores a UI configuration state snapshot. */
    void setUIConfigState(const std::unique_ptr<juce::XmlElement>& uiConfigState);
    /** @brief Returns the cached UI configuration state. */
    const std::unique_ptr<juce::XmlElement>& getUIConfigState();

    /** @brief Opens a file chooser dialog to load a configuration file. */
    void triggerPromptLoadConfig();
    /** @brief Opens a file chooser dialog to save the configuration file. */
    void triggerPromptSaveConfig();

    JUCE_DECLARE_SINGLETON(Mema, false)

private:
    std::unique_ptr<MemaProcessor>          m_MemaProcessor;            ///< The core audio processor instance.
    std::unique_ptr<juce::XmlElement>       m_MemaUIConfigCache;        ///< Cached UI configuration XML.

    std::unique_ptr<MemaProcessorEditor>    m_audioVisuComponent;       ///< The processor editor/visualizer component.
    std::unique_ptr<AudioSelectComponent>   m_audioDeviceSelectComponent; ///< Audio device selection component.

    std::unique_ptr<MemaAppConfiguration>   m_config;                   ///< Application configuration manager.

    std::unique_ptr<juce::FileChooser>      m_loadSavefileChooser;      ///< File chooser for load/save operations.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mema)
};

};

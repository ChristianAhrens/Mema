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

#include "Mema.h"

#include "MemaProcessorEditor/MemaProcessorEditor.h"
#include "MemaProcessor/MemaProcessor.h"

#include <WebUpdateDetector.h>

namespace Mema
{

//==============================================================================
Mema::Mema() :  juce::Timer()
{
    // create the configuration object (is being initialized from disk automatically)
    m_config = std::make_unique<AppConfiguration>(JUCEAppBasics::AppConfigurationBase::getDefaultConfigFilePath());
    m_config->addDumper(this);

    // check if config creation was able to read a valid config from disk...
    if (!m_config->isValid())
    {
        m_config->ResetToDefault();
    }

    // add this main component to watchers
    m_config->addWatcher(this, true); // this initial update cannot yet reach all parts of the app, esp. settings page that relies on fully initialized pagecomponentmanager, therefor a manual watcher update is triggered below

    m_MemaProcessor = std::make_unique<MemaProcessor>(m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORCONFIG)).get());

    m_audioDeviceSelectComponent = std::make_unique<AudioSelectComponent>(m_MemaProcessor->getDeviceManager(),
                                                                          MemaProcessor::s_minInputsCount,
                                                                          MemaProcessor::s_maxChannelCount,
                                                                          MemaProcessor::s_minOutputsCount,
                                                                          MemaProcessor::s_maxChannelCount,
                                                                          false, false, false, false);

    // do the initial update for the whole application with config contents
    m_config->triggerWatcherUpdate();

    startTimer(500);

#if defined JUCE_IOS
// iOS is updated via AppStore
#define IGNORE_UPDATES
#elif defined JUCE_ANDROID
// Android as well
#define IGNORE_UPDATES
#endif

#if defined IGNORE_UPDATES
#else
    auto updater = JUCEAppBasics::WebUpdateDetector::getInstance();
    updater->SetReferenceVersion(ProjectInfo::versionString);
    updater->SetDownloadUpdateWebAddress("https://github.com/christianahrens/mema/releases/latest");
    updater->CheckForNewVersion(true, "https://raw.githubusercontent.com/ChristianAhrens/Mema/refs/heads/main/");
#endif
}

Mema::~Mema()
{
    if (m_MemaProcessor)
        m_MemaProcessor->editorBeingDeleted(m_MemaProcessor->getActiveEditor());
}

void Mema::timerCallback()
{
    if (m_MemaProcessor && m_MemaProcessor->getDeviceManager())
    {
        if (onCpuUsageUpdate)
            onCpuUsageUpdate(int(m_MemaProcessor->getDeviceManager()->getCpuUsage() * 100.0));
        if (onNetworkUsageUpdate)
            onNetworkUsageUpdate(m_MemaProcessor->getNetworkHealth());
    }
}

juce::Component* Mema::getMemaProcessorEditor()
{
    if (m_MemaProcessor)
    {
        if (nullptr == m_MemaProcessor->getActiveEditor())
            m_MemaProcessor->createEditorIfNeeded();

        if (auto editor = dynamic_cast<MemaProcessorEditor*>(m_MemaProcessor->getActiveEditor()))
        {
            jassert(onEditorSizeChangeRequested); // should be set before handling the ui component!
            editor->onEditorSizeChangeRequested = onEditorSizeChangeRequested;
        }

        return m_MemaProcessor->getActiveEditor();
    }
    else
        return nullptr;
}

juce::Component* Mema::getDeviceSetupComponent()
{
    if (m_audioDeviceSelectComponent)
        return m_audioDeviceSelectComponent.get();
    else
        return nullptr;
}

void Mema::performConfigurationDump()
{
    if (m_config)
    {
        auto stateXml = m_config->getConfigState();

        if (stateXml && m_MemaProcessor)
        {
            m_config->setConfigState(m_MemaProcessor->createStateXml(), AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORCONFIG));

            if (auto editor = dynamic_cast<MemaProcessorEditor*>(m_MemaProcessor->getActiveEditor()))
            {
                m_config->setConfigState(editor->createStateXml(), AppConfiguration::getTagName(AppConfiguration::TagID::EDITORCONFIG));
            }
        }
    }
}

void Mema::onConfigUpdated()
{
    auto processorConfigState = m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORCONFIG));
    if (processorConfigState && m_MemaProcessor)
    {
        m_MemaProcessor->setStateXml(processorConfigState.get());
    }
        
    auto editorConfigState = m_config->getConfigState(AppConfiguration::getTagName(AppConfiguration::TagID::EDITORCONFIG));
    if (editorConfigState && m_MemaProcessor && m_MemaProcessor->getActiveEditor())
    {
        if (auto editor = dynamic_cast<MemaProcessorEditor*>(m_MemaProcessor->getActiveEditor()))
        {
            editor->setStateXml(editorConfigState.get());
        }
    }
}

void Mema::propagateLookAndFeelChanged()
{
    if (m_MemaProcessor)
    {
        m_MemaProcessor->environmentChanged();
        
        if (auto editor = dynamic_cast<MemaProcessorEditor*>(m_MemaProcessor->getActiveEditor()))
        {
            editor->lookAndFeelChanged();
        }
    }
}


}

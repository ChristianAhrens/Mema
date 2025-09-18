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


JUCE_IMPLEMENT_SINGLETON(Mema)

//==============================================================================
Mema::Mema() :  juce::Timer()
{
    // create the configuration object (is being initialized from disk automatically)
    m_config = std::make_unique<MemaAppConfiguration>(JUCEAppBasics::AppConfigurationBase::getDefaultConfigFilePath());
    m_config->addDumper(this);

    // check if config creation was able to read a valid config from disk...
    if (!m_config->isValid())
    {
        m_config->ResetToDefault();
    }

    // add this main component to watchers
    m_config->addWatcher(this, true); // this initial update cannot yet reach all parts of the app, esp. settings page that relies on fully initialized pagecomponentmanager, therefor a manual watcher update is triggered below

    m_MemaProcessor = std::make_unique<MemaProcessor>(m_config->getConfigState(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::PROCESSORCONFIG)).get());

    m_audioDeviceSelectComponent = std::make_unique<AudioSelectComponent>(m_MemaProcessor->getDeviceManager(),
                                                                          MemaProcessor::s_minInputsCount,
                                                                          MemaProcessor::s_maxChannelCount,
                                                                          MemaProcessor::s_minOutputsCount,
                                                                          MemaProcessor::s_maxChannelCount,
                                                                          false, false, false, false);
    m_audioDeviceSelectComponent->onAudioDeviceChangedDuringAudioSelection = [=]() {
        if (m_MemaProcessor)
            m_MemaProcessor->initializeCtrlValuesToUnity();
    };

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

        m_MemaProcessor->updateCommanders();

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

void Mema::clearUICallbacks()
{
    onEditorSizeChangeRequested = nullptr;
    onCpuUsageUpdate = nullptr;
    onNetworkUsageUpdate = nullptr;
    
    if (auto editor = dynamic_cast<MemaProcessorEditor*>(m_MemaProcessor->getActiveEditor()))
        editor->onEditorSizeChangeRequested = nullptr;
}

void Mema::performConfigurationDump()
{
    if (m_config)
    {
        auto stateXml = m_config->getConfigState();
        if (stateXml)
        {
            if (m_MemaProcessor)
                m_config->setConfigState(m_MemaProcessor->createStateXml(), MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::PROCESSORCONFIG));
            if (m_MemaUIConfigCache)
                m_config->setConfigState(std::make_unique<juce::XmlElement>(*m_MemaUIConfigCache), MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::UICONFIG));
        }
    }
}

void Mema::onConfigUpdated()
{
    auto processorConfigState = m_config->getConfigState(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::PROCESSORCONFIG));
    if (processorConfigState && m_MemaProcessor)
    {
        m_MemaProcessor->setStateXml(processorConfigState.get());
    }
        
    auto uiConfigState = m_config->getConfigState(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::UICONFIG));
    if (uiConfigState && m_MemaProcessor)
    {
        m_MemaUIConfigCache = std::make_unique<juce::XmlElement>(*uiConfigState);
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

void Mema::setUIConfigState(const std::unique_ptr<juce::XmlElement>& uiConfigState)
{
    m_MemaUIConfigCache = std::make_unique<juce::XmlElement>(*uiConfigState);
}

const std::unique_ptr<juce::XmlElement>& Mema::getUIConfigState()
{
    return m_MemaUIConfigCache;
}

void Mema::triggerPromptLoadConfig()
{
    m_loadSavefileChooser = std::make_unique<juce::FileChooser>(
        "Please select the " + juce::JUCEApplication::getInstance()->getApplicationName() + " configuration file you want to load...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory),
        "*.config");
    
    m_loadSavefileChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [=](const juce::FileChooser& chooser) {
            juce::File sourceFile(chooser.getResult());
            
            if (!sourceFile.existsAsFile() || !sourceFile.hasReadAccess())
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Loading failed")
                    .withMessage("The file " + sourceFile.getFileName() + " cannot be accessed for reading.")
                    .withButton("Ok"), nullptr);
                return false;
            }

            auto config = MemaAppConfiguration::getInstance();
            if (!config)
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Loading failed")
                    .withMessage("There was an internal error with the configuration.")
                    .withButton("Ok"), nullptr);
                return false;
            }

            auto xmlConfig = juce::parseXML(sourceFile);
            if (!xmlConfig)
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Loading failed")
                    .withMessage("The file " + sourceFile.getFileName() + " has invalid contents.")
                    .withButton("Ok"), nullptr);
                return false;
            }

            if (!MemaAppConfiguration::isValid(xmlConfig))
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Loading failed")
                    .withMessage("The file " + sourceFile.getFileName() + " has invalid contents.")
                    .withButton("Ok"), nullptr);
                return false;
            }

            config->SetFlushAndUpdateDisabled();
            if (!config->resetConfigState(std::move(xmlConfig)))
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Loading failed")
                    .withMessage("There was an internal error with applying the configuration.")
                    .withButton("Ok"), nullptr);
                config->ResetFlushAndUpdateDisabled();
                return false;
            }
            config->ResetFlushAndUpdateDisabled();

            return true;
        });
}

void Mema::triggerPromptSaveConfig()
{
    m_loadSavefileChooser = std::make_unique<juce::FileChooser>(
        "Please select the " + juce::JUCEApplication::getInstance()->getApplicationName() + " configuration file target you want to save to...",
        juce::File::getSpecialLocation(juce::File::userHomeDirectory).getChildFile(
            juce::Time::getCurrentTime().toISO8601(true).substring(0, 10) + "_" +
            juce::JUCEApplication::getInstance()->getApplicationName() + ".config"),
        "*.config");

    m_loadSavefileChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [=](const juce::FileChooser& chooser) {
            juce::File targetFile(chooser.getResult());

            // enforce the .config extension
            if (targetFile.getFileExtension() != ".config")
                targetFile = targetFile.withFileExtension(".config");

            if (!targetFile.hasWriteAccess())
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Saving failed")
                    .withMessage("The file " + targetFile.getFileName() + " cannot be accessed for writing.")
                    .withButton("Ok"), nullptr);
                return false;
            }

            auto config = MemaAppConfiguration::getInstance();
            if (!config)
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Saving failed")
                    .withMessage("There was an internal error with the configuration (0x0).")
                    .withButton("Ok"), nullptr);
                return false;
            }

            auto xmlConfig = config->getConfigState();
            if (!xmlConfig)
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Saving failed")
                    .withMessage("There was an internal error with the configuration (0x1).")
                    .withButton("Ok"), nullptr);
                return false;
            }
            else if (!xmlConfig->writeTo(targetFile))
            {
                juce::AlertWindow::showAsync(juce::MessageBoxOptions()
                    .withIconType(juce::MessageBoxIconType::WarningIcon)
                    .withTitle("Saving failed")
                    .withMessage("There was an error when writing the configuration to disk.")
                    .withButton("Ok"), nullptr);
                return false;
            }
            else
                return true;
        });
}


}

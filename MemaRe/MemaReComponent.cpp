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

#include "MemaReComponent.h"

#include <MemaProcessor/MemaMessages.h>
#include <MemaProcessor/MemaProcessor.h>

MemaReComponent::MemaReComponent()
    : juce::Component()
{
    m_faderbankCtrlComponent = std::make_unique<Mema::FaderbankControlComponent>();
    m_faderbankCtrlComponent->onInputMutesChanged = [=](const std::map<std::uint16_t, bool>& inputMuteStates) {
        std::map<std::uint16_t, bool> outputMuteStates;
        std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
        std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates, crosspointValues)->getSerializedMessage());
    };
    m_faderbankCtrlComponent->onOutputMutesChanged = [=](const std::map<std::uint16_t, bool>& outputMuteStates) {
        std::map<std::uint16_t, bool> inputMuteStates;
        std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
        std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates, crosspointValues)->getSerializedMessage());
    };
    m_faderbankCtrlComponent->onCrosspointStatesChanged = [=](const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates) {
        std::map<std::uint16_t, bool> inputMuteStates;
        std::map<std::uint16_t, bool> outputMuteStates;
        std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates, crosspointValues)->getSerializedMessage());
    };
    m_faderbankCtrlComponent->onCrosspointValuesChanged = [=](const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues) {
        std::map<std::uint16_t, bool> inputMuteStates;
        std::map<std::uint16_t, bool> outputMuteStates;
        std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates, crosspointValues)->getSerializedMessage());
    };
    addChildComponent(m_faderbankCtrlComponent.get());

    m_panningCtrlComponent = std::make_unique<Mema::PanningControlComponent>();
    m_panningCtrlComponent->onInputMutesChanged = [=](const std::map<std::uint16_t, bool>& inputMuteStates) {
        std::map<std::uint16_t, bool> outputMuteStates;
        std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
        std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates, crosspointValues)->getSerializedMessage());
        };
    m_panningCtrlComponent->onOutputMutesChanged = [=](const std::map<std::uint16_t, bool>& outputMuteStates) {
        std::map<std::uint16_t, bool> inputMuteStates;
        std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
        std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates, crosspointValues)->getSerializedMessage());
        };
    m_panningCtrlComponent->onCrosspointStatesChanged = [=](const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates) {
        std::map<std::uint16_t, bool> inputMuteStates;
        std::map<std::uint16_t, bool> outputMuteStates;
        std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates, crosspointValues)->getSerializedMessage());
        };
    m_panningCtrlComponent->onCrosspointValuesChanged = [=](const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues) {
        std::map<std::uint16_t, bool> inputMuteStates;
        std::map<std::uint16_t, bool> outputMuteStates;
        std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates, crosspointValues)->getSerializedMessage());
        };
    m_panningCtrlComponent->setExternalControlSettings(std::get<0>(m_externalAdmOscSettings), std::get<1>(m_externalAdmOscSettings), std::get<2>(m_externalAdmOscSettings));
    addChildComponent(m_panningCtrlComponent.get());

    m_pluginCtrlComponent = std::make_unique<Mema::PluginControlComponent>();
    m_pluginCtrlComponent->onPluginParameterValueChanged = [=](std::uint16_t parameterIndex, std::string parameterId, float value) {
        DBG(juce::String(__FUNCTION__) + " sending to net (" + juce::String(parameterIndex) + "; " + juce::String(parameterId) + "; " + juce::String(value) + ") ...");
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::PluginParameterValueMessage>(parameterIndex, parameterId, value)->getSerializedMessage());
        };
    m_pluginCtrlComponent->onPluginEnabledChanged = [=](bool enabled) {
        m_pluginEnabled = enabled;
        DBG(juce::String(__FUNCTION__) + " sending enabled state to net: " + juce::String(int(enabled)));
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::PluginProcessingStateMessage>(m_pluginEnabled, m_pluginPost)->getSerializedMessage());
        };
    m_pluginCtrlComponent->onPluginPrePostChanged = [=](bool post) {
        m_pluginPost = post;
        DBG(juce::String(__FUNCTION__) + " sending pre/post state to net: " + juce::String(int(post)));
        if (onMessageReadyToSend)
            onMessageReadyToSend(std::make_unique<Mema::PluginProcessingStateMessage>(m_pluginEnabled, m_pluginPost)->getSerializedMessage());
        };
    addChildComponent(m_pluginCtrlComponent.get());

    setFaderbankCtrlActive();
}

MemaReComponent::~MemaReComponent()
{
}

void MemaReComponent::setFaderbankCtrlActive()
{
    auto resizeRequired = false;
    
    if (m_faderbankCtrlComponent)
    {
        if (!m_faderbankCtrlComponent->isVisible())
        {
            m_faderbankCtrlComponent->setIOCount(m_currentIOCount);
            m_faderbankCtrlComponent->setVisible(true);
            resizeRequired = true;
        }
    }
    if (m_panningCtrlComponent && m_panningCtrlComponent->isVisible())
    {
        m_panningCtrlComponent->resetCtrl();
        m_panningCtrlComponent->setVisible(false);
        resizeRequired = true;
    }
    if (m_pluginCtrlComponent && m_pluginCtrlComponent->isVisible())
    {
        m_pluginCtrlComponent->resetCtrl();
        m_pluginCtrlComponent->setVisible(false);
        resizeRequired = true;
    }

    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaReComponent::setOutputPanningCtrlActive(const juce::AudioChannelSet& channelConfiguration)
{
    auto resizeRequired = false;

    if (m_panningCtrlComponent)
    {
        if (!m_panningCtrlComponent->isVisible())
        {
            m_panningCtrlComponent->setIOCount(m_currentIOCount);
            m_panningCtrlComponent->setVisible(true);
            if (!m_inputMuteStates.empty())
                m_panningCtrlComponent->setInputMuteStates(m_inputMuteStates);
            if (!m_outputMuteStates.empty())
                m_panningCtrlComponent->setOutputMuteStates(m_outputMuteStates);
            if (!m_crosspointStates.empty())
                m_panningCtrlComponent->setCrosspointStates(m_crosspointStates);
            if (!m_crosspointValues.empty())
                m_panningCtrlComponent->setCrosspointValues(m_crosspointValues);
            resizeRequired = true;
        }
        m_panningCtrlComponent->setChannelConfig(channelConfiguration);
    }
    if (m_faderbankCtrlComponent && m_faderbankCtrlComponent->isVisible())
    {
        m_faderbankCtrlComponent->resetCtrl();
        m_faderbankCtrlComponent->setVisible(false);
        resizeRequired = true;
    }
    if (m_pluginCtrlComponent && m_pluginCtrlComponent->isVisible())
    {
        m_pluginCtrlComponent->resetCtrl();
        m_pluginCtrlComponent->setVisible(false);
        resizeRequired = true;
    }

    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaReComponent::setPluginCtrlActive()
{
    auto resizeRequired = false;

    if (m_pluginCtrlComponent)
    {
        if (!m_pluginCtrlComponent->isVisible())
        {
            m_pluginCtrlComponent->setVisible(true);
            resizeRequired = true;
        }
        // Apply any data that arrived before the component was made visible
        if (!m_pluginName.empty() || !m_pluginParameterInfos.empty())
        {
            m_pluginCtrlComponent->setPluginName(m_pluginName);
            m_pluginCtrlComponent->setParameterInfos(m_pluginParameterInfos);
        }
        m_pluginCtrlComponent->setPluginEnabled(m_pluginEnabled);
        m_pluginCtrlComponent->setPluginPrePost(m_pluginPost);
    }
    if (m_faderbankCtrlComponent && m_faderbankCtrlComponent->isVisible())
    {
        m_faderbankCtrlComponent->resetCtrl();
        m_faderbankCtrlComponent->setVisible(false);
        resizeRequired = true;
    }
    if (m_panningCtrlComponent && m_panningCtrlComponent->isVisible())
    {
        m_panningCtrlComponent->resetCtrl();
        m_panningCtrlComponent->setVisible(false);
        resizeRequired = true;
    }

    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaReComponent::resetCtrl()
{
    m_pluginName.clear();
    m_pluginParameterInfos.clear();
    m_pluginEnabled = false;
    m_pluginPost = false;
    m_inputMuteStates.clear();
    m_outputMuteStates.clear();
    m_crosspointStates.clear();
    m_crosspointValues.clear();

    if (m_faderbankCtrlComponent)
        m_faderbankCtrlComponent->resetCtrl();
    if (m_panningCtrlComponent)
        m_panningCtrlComponent->resetCtrl();
    if (m_pluginCtrlComponent)
        m_pluginCtrlComponent->resetCtrl();
}

void MemaReComponent::setControlsSize(const Mema::MemaClientControlComponentBase::ControlsSize& ctrlsSize)
{
    if (m_faderbankCtrlComponent)
        m_faderbankCtrlComponent->setControlsSize(ctrlsSize);
    if (m_panningCtrlComponent)
        m_panningCtrlComponent->setControlsSize(ctrlsSize);
    if (m_pluginCtrlComponent)
        m_pluginCtrlComponent->setControlsSize(ctrlsSize);
}

const Mema::MemaClientControlComponentBase::ControlsSize MemaReComponent::getControlsSize()
{
    if (m_faderbankCtrlComponent)
        return m_faderbankCtrlComponent->getControlsSize();
    else if (m_panningCtrlComponent)
        return m_panningCtrlComponent->getControlsSize();
    else if (m_pluginCtrlComponent)
        return m_pluginCtrlComponent->getControlsSize();
    else
        return Mema::MemaClientControlComponentBase::ControlsSize::S;
}

void MemaReComponent::setExternalAdmOscSettings(const int ADMOSCport, const juce::IPAddress& ADMOSCremoteIP, const int ADMOSCremotePort)
{
    std::get<0>(m_externalAdmOscSettings) = ADMOSCport;
    std::get<1>(m_externalAdmOscSettings) = ADMOSCremoteIP;
    std::get<2>(m_externalAdmOscSettings) = ADMOSCremotePort;

    m_panningCtrlComponent->setExternalControlSettings(ADMOSCport, ADMOSCremoteIP, ADMOSCremotePort);
}

std::tuple<int, juce::IPAddress, int> MemaReComponent::getExternalAdmOscSettings()
{
    return m_externalAdmOscSettings;
}

void MemaReComponent::paint(Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::widgetBackground));
}

void MemaReComponent::resized()
{
    if (m_faderbankCtrlComponent && m_faderbankCtrlComponent->isVisible())
        m_faderbankCtrlComponent->setBounds(getLocalBounds());
    if (m_panningCtrlComponent && m_panningCtrlComponent->isVisible())
        m_panningCtrlComponent->setBounds(getLocalBounds());
    if (m_pluginCtrlComponent && m_pluginCtrlComponent->isVisible())
        m_pluginCtrlComponent->setBounds(getLocalBounds());
}

void MemaReComponent::handleMessage(const Message& message)
{
    if (RunningStatus::Active != m_runningStatus)
    {
        m_runningStatus = RunningStatus::Active;
        resized();
    }
    
    if (auto const apm = dynamic_cast<const Mema::AnalyzerParametersMessage*>(&message))
    {
        // we don't want analyzer parameter infos!
        DBG(juce::String(__FUNCTION__) + " ignoring unexpected AnalyzerParametersMessage...");
    }
    else if (auto m = dynamic_cast<const Mema::AudioBufferMessage*>(&message))
    {
        // we don't want audio buffer infos!
        DBG(juce::String(__FUNCTION__) + " ignoring unexpected AudiBufferMessage...");
    }
    else if (auto const iom = dynamic_cast<const Mema::ReinitIOCountMessage*>(&message))
    {
        DBG(juce::String(__FUNCTION__) + " handling ReinitIOCountMessage...");

        auto inputCount = iom->getInputCount();
        jassert(inputCount > 0);
        auto outputCount = iom->getOutputCount();
        jassert(outputCount > 0);

        m_currentIOCount = std::make_pair(inputCount, outputCount);

        if (m_faderbankCtrlComponent)
            m_faderbankCtrlComponent->setIOCount(m_currentIOCount);
        if (m_panningCtrlComponent && m_panningCtrlComponent->isVisible())
            m_panningCtrlComponent->setIOCount(m_currentIOCount);

        resized();
    }
    else if (auto const cpm = dynamic_cast<const Mema::ControlParametersMessage*>(&message))
    {
        DBG(juce::String(__FUNCTION__) + " handling ControlParametersMessage...");

        for (auto const& inputMuteState : cpm->getInputMuteStates())
            m_inputMuteStates[inputMuteState.first] = inputMuteState.second;
        if (!m_inputMuteStates.empty())
        {
            if (m_faderbankCtrlComponent)
                m_faderbankCtrlComponent->setInputMuteStates(m_inputMuteStates);
            if (m_panningCtrlComponent && m_panningCtrlComponent->isVisible())
                m_panningCtrlComponent->setInputMuteStates(m_inputMuteStates);
        }

        for (auto const& outputMuteState : cpm->getOutputMuteStates())
            m_outputMuteStates[outputMuteState.first] = outputMuteState.second;
        if (!m_outputMuteStates.empty())
        {
            if (m_faderbankCtrlComponent)
                m_faderbankCtrlComponent->setOutputMuteStates(m_outputMuteStates);
            if (m_panningCtrlComponent && m_panningCtrlComponent->isVisible())
                m_panningCtrlComponent->setOutputMuteStates(m_outputMuteStates);
        }

        for (auto const& cpsIKV : cpm->getCrosspointStates())
            for (auto const& cpsOKV : cpsIKV.second)
                m_crosspointStates[cpsIKV.first][cpsOKV.first] = cpsOKV.second;
        if (!m_crosspointStates.empty())
        {
            if (m_faderbankCtrlComponent)
                m_faderbankCtrlComponent->setCrosspointStates(m_crosspointStates);
            if (m_panningCtrlComponent && m_panningCtrlComponent->isVisible())
                m_panningCtrlComponent->setCrosspointStates(m_crosspointStates);
        }

        for (auto const& cpvIKV : cpm->getCrosspointValues())
            for (auto const& cpvOKV : cpvIKV.second)
                m_crosspointValues[cpvIKV.first][cpvOKV.first] = cpvOKV.second;
        if (!m_crosspointValues.empty())
        {
            if (m_faderbankCtrlComponent)
                m_faderbankCtrlComponent->setCrosspointValues(m_crosspointValues);
            if (m_panningCtrlComponent && m_panningCtrlComponent->isVisible())
                m_panningCtrlComponent->setCrosspointValues(m_crosspointValues);
        }

        resized();
    }
    else if (auto const ppim = dynamic_cast<const Mema::PluginParameterInfosMessage*>(&message))
    {
        DBG(juce::String(__FUNCTION__) + " handling PluginParameterInfosMessage (" + juce::String(ppim->getParameterInfos().size()) + ") ...");

        m_pluginName = ppim->getPluginName().toStdString();
        m_pluginParameterInfos = ppim->getParameterInfos();
        m_pluginEnabled = ppim->isPluginEnabled();
        m_pluginPost = ppim->isPluginPost();

        if (m_pluginCtrlComponent && m_pluginCtrlComponent->isVisible())
        {
            m_pluginCtrlComponent->setPluginName(m_pluginName);
            m_pluginCtrlComponent->setParameterInfos(m_pluginParameterInfos);
            m_pluginCtrlComponent->setPluginEnabled(m_pluginEnabled);
            m_pluginCtrlComponent->setPluginPrePost(m_pluginPost);
        }

        resized();
    }
    else if (auto const pesm = dynamic_cast<const Mema::PluginProcessingStateMessage*>(&message))
    {
        DBG(juce::String(__FUNCTION__) + " handling PluginProcessingStateMessage (enabled:" + juce::String(int(pesm->isEnabled())) + " post:" + juce::String(int(pesm->isPost())) + ") ...");

        m_pluginEnabled = pesm->isEnabled();
        m_pluginPost = pesm->isPost();

        if (m_pluginCtrlComponent && m_pluginCtrlComponent->isVisible())
        {
            m_pluginCtrlComponent->setPluginEnabled(m_pluginEnabled);
            m_pluginCtrlComponent->setPluginPrePost(m_pluginPost);
        }
    }
    else if (auto const ppvm = dynamic_cast<const Mema::PluginParameterValueMessage*>(&message))
    {
        DBG(juce::String(__FUNCTION__) + " handling PluginParameterValueMessage (" + juce::String(ppvm->getParameterIndex()) + "; " + ppvm->getParameterId() + "; " + juce::String(ppvm->getCurrentValue()) + ") ...");

        auto paramIdx = ppvm->getParameterIndex();
        if (paramIdx < m_pluginParameterInfos.size())
        {
            m_pluginParameterInfos[paramIdx].currentValue = ppvm->getCurrentValue();
            m_pluginParameterInfos[paramIdx].id = ppvm->getParameterId();
        }

        if (m_pluginCtrlComponent && m_pluginCtrlComponent->isVisible())
            m_pluginCtrlComponent->setParameterValue(paramIdx, ppvm->getParameterId().toStdString(), ppvm->getCurrentValue());

        resized();
    }
}


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

#include "MemaClientCommon/MemaClientControlComponents.h"

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
    addChildComponent(m_panningCtrlComponent.get());

    setOutputFaderbankCtrlActive();
}

MemaReComponent::~MemaReComponent()
{
}

void MemaReComponent::setOutputFaderbankCtrlActive()
{
    auto resizeRequired = false;
    
    if (m_faderbankCtrlComponent && !m_faderbankCtrlComponent->isVisible())
    {
        m_faderbankCtrlComponent->setVisible(true);
        resizeRequired = true;
    }
    if (m_panningCtrlComponent && m_panningCtrlComponent->isVisible())
    {
        m_panningCtrlComponent->setVisible(false);
        resizeRequired = true;
    }

    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaReComponent::setOutputPanningCtrlActive(const juce::AudioChannelSet& channelConfiguration)
{
    auto resizeRequired = false;

    if (m_panningCtrlComponent && !m_panningCtrlComponent->isVisible())
    {
        m_panningCtrlComponent->setChannelConfig(channelConfiguration);
        m_panningCtrlComponent->setVisible(true);
        resizeRequired = true;
    }
    if (m_faderbankCtrlComponent && m_faderbankCtrlComponent->isVisible())
    {
        m_faderbankCtrlComponent->setVisible(false);
        resizeRequired = true;
    }

    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaReComponent::resetCtrl()
{
    if (m_faderbankCtrlComponent)
        m_faderbankCtrlComponent->resetCtrl();
    if (m_panningCtrlComponent)
        m_panningCtrlComponent->resetCtrl();
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
        auto inputCount = iom->getInputCount();
        jassert(inputCount > 0);
        auto outputCount = iom->getOutputCount();
        jassert(outputCount > 0);

        m_currentIOCount = std::make_pair(inputCount, outputCount);

        if (m_faderbankCtrlComponent)
            m_faderbankCtrlComponent->setIOCount(m_currentIOCount);
        if (m_panningCtrlComponent)
            m_panningCtrlComponent->setIOCount(m_currentIOCount);

        resized();
    }
    else if (auto const cpm = dynamic_cast<const Mema::ControlParametersMessage*>(&message))
    {
        for (auto const& inputMuteState : cpm->getInputMuteStates())
            m_inputMuteStates[inputMuteState.first] = inputMuteState.second;
        if (!m_inputMuteStates.empty())
        {
            if (m_faderbankCtrlComponent)
                m_faderbankCtrlComponent->setInputMuteStates(m_inputMuteStates);
            if (m_panningCtrlComponent)
                m_panningCtrlComponent->setInputMuteStates(m_inputMuteStates);
        }

        for (auto const& outputMuteState : cpm->getOutputMuteStates())
            m_outputMuteStates[outputMuteState.first] = outputMuteState.second;
        if (!m_outputMuteStates.empty())
        {
            if (m_faderbankCtrlComponent)
                m_faderbankCtrlComponent->setOutputMuteStates(m_outputMuteStates);
            if (m_panningCtrlComponent)
                m_panningCtrlComponent->setOutputMuteStates(m_outputMuteStates);
        }

        for (auto const& cpsIKV : cpm->getCrosspointStates())
            for (auto const& cpsOKV : cpsIKV.second)
                m_crosspointStates[cpsIKV.first][cpsOKV.first] = cpsOKV.second;
        if (!m_crosspointStates.empty())
        {
            if (m_faderbankCtrlComponent)
                m_faderbankCtrlComponent->setCrosspointStates(m_crosspointStates);
            if (m_panningCtrlComponent)
                m_panningCtrlComponent->setCrosspointStates(m_crosspointStates);
        }

        for (auto const& cpvIKV : cpm->getCrosspointValues())
            for (auto const& cpvOKV : cpvIKV.second)
                m_crosspointValues[cpvIKV.first][cpvOKV.first] = cpvOKV.second;
        if (!m_crosspointValues.empty())
        {
            if (m_faderbankCtrlComponent)
                m_faderbankCtrlComponent->setCrosspointValues(m_crosspointValues);
            if (m_panningCtrlComponent)
                m_panningCtrlComponent->setCrosspointValues(m_crosspointValues);
        }

        resized();
    }
}


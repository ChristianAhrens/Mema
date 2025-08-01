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

#include "MemaMoComponent.h"

#include "MemaProcessorEditor/MeterbridgeComponent.h"
#include "MemaClientCommon/TwoDFieldOutputComponent.h"
#include "MemaClientCommon/WaveformAudioComponent.h"
#include "MemaProcessor/MemaMessages.h"
#include "MemaProcessor/MemaProcessor.h"
#include "MemaProcessor/ProcessorDataAnalyzer.h"

MemaMoComponent::MemaMoComponent()
    : juce::Component()
{
    m_inputMeteringComponent = std::make_unique<Mema::MeterbridgeComponent>(Mema::MeterbridgeComponent::Direction::Horizontal);
    addAndMakeVisible(m_inputMeteringComponent.get());

    m_inputDataAnalyzer = std::make_unique<Mema::ProcessorDataAnalyzer>();
    m_inputDataAnalyzer->addListener(m_inputMeteringComponent.get());

    m_outputDataAnalyzer = std::make_unique<Mema::ProcessorDataAnalyzer>();

    setOutputMeteringVisuActive();
}

MemaMoComponent::~MemaMoComponent()
{
}

void MemaMoComponent::setOutputMeteringVisuActive()
{
    auto resizeRequired = false;
    // if ioMeter should be visualized, make sure the components existis
    if (!m_inputMeteringComponent)
    {
        m_inputMeteringComponent = std::make_unique<Mema::MeterbridgeComponent>(Mema::MeterbridgeComponent::Direction::Horizontal);
        m_inputMeteringComponent->setChannelCount(m_currentIOCount.first);
        addAndMakeVisible(m_inputMeteringComponent.get());
        if (m_inputDataAnalyzer)
            m_inputDataAnalyzer->addListener(m_inputMeteringComponent.get());
        resizeRequired = true;
    }
    if (!m_outputMeteringComponent)
    {
        m_outputMeteringComponent = std::make_unique<Mema::MeterbridgeComponent>(Mema::MeterbridgeComponent::Direction::Horizontal);
        m_outputMeteringComponent->setChannelCount(m_currentIOCount.second);
        addAndMakeVisible(m_outputMeteringComponent.get());
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->addListener(m_outputMeteringComponent.get());
        resizeRequired = true;
    }

    // none of the other components outputfields/waveF are required - cleanup
    if (m_outputFieldComponent)
    {
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->removeListener(m_outputFieldComponent.get());

        removeChildComponent(m_outputFieldComponent.get());
        m_outputFieldComponent.reset();
        resizeRequired = true;
    }

    if (m_waveformComponent)
    {
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->removeListener(m_waveformComponent.get());

        removeChildComponent(m_waveformComponent.get());
        m_waveformComponent.reset();
        resizeRequired = true;
    }

    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaMoComponent::setOutputFieldVisuActive(const juce::AudioChannelSet& channelConfiguration)
{
    auto resizeRequired = false;
    // if outputfields (incl. iMeter) should be visualized, make sure the components existis
    if (!m_inputMeteringComponent)
    {
        m_inputMeteringComponent = std::make_unique<Mema::MeterbridgeComponent>(Mema::MeterbridgeComponent::Direction::Horizontal);
        m_inputMeteringComponent->setChannelCount(m_currentIOCount.first);
        addAndMakeVisible(m_inputMeteringComponent.get());
        if (m_inputDataAnalyzer)
            m_inputDataAnalyzer->addListener(m_inputMeteringComponent.get());
        resizeRequired = true;
    }
    if (!m_outputFieldComponent)
    {
        m_outputFieldComponent = std::make_unique<Mema::TwoDFieldOutputComponent>();
        addAndMakeVisible(m_outputFieldComponent.get());
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->addListener(m_outputFieldComponent.get());
        resizeRequired = true;
    }
    if (m_outputFieldComponent->setChannelConfiguration(channelConfiguration))
        resizeRequired = true;

    // none of the other components oMeter/waveF are required - cleanup
    if (m_outputMeteringComponent)
    {
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->removeListener(m_outputMeteringComponent.get());

        removeChildComponent(m_outputMeteringComponent.get());
        m_outputMeteringComponent.reset();
        resizeRequired = true;
    }

    if (m_waveformComponent)
    {
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->removeListener(m_waveformComponent.get());

        removeChildComponent(m_waveformComponent.get());
        m_waveformComponent.reset();
        resizeRequired = true;
    }

    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaMoComponent::setWaveformVisuActive()
{
    auto resizeRequired = false;
    // if waveform should be visualized, make sure the component existis
    if (!m_waveformComponent)
    {
        m_waveformComponent = std::make_unique<Mema::WaveformAudioComponent>();
        addAndMakeVisible(m_waveformComponent.get());
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->addListener(m_waveformComponent.get());
        resizeRequired = true;
    }

    // none of the other components ioMeter/outpField are required - cleanup
    if (m_inputMeteringComponent)
    {
        if (m_inputDataAnalyzer)
            m_inputDataAnalyzer->removeListener(m_inputMeteringComponent.get());

        removeChildComponent(m_inputMeteringComponent.get());
        m_inputMeteringComponent.reset();
        resizeRequired = true;
    }

    if (m_outputMeteringComponent)
    {
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->removeListener(m_outputMeteringComponent.get());

        removeChildComponent(m_outputMeteringComponent.get());
        m_outputMeteringComponent.reset();
        resizeRequired = true;
    }

    if (m_outputFieldComponent)
    {
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->removeListener(m_outputFieldComponent.get());

        removeChildComponent(m_outputFieldComponent.get());
        m_outputFieldComponent.reset();
        resizeRequired = true;
    }
    
    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaMoComponent::paint(Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::widgetBackground));
}

void MemaMoComponent::resized()
{
    if (m_inputMeteringComponent && m_outputFieldComponent)
    {
        auto margin = 8;
        auto bounds = getLocalBounds().reduced(margin, margin);
        auto boundsAspect = bounds.toFloat().getAspectRatio();
        auto fieldAspect = m_outputFieldComponent->getRequiredAspectRatio();
        if (boundsAspect >= 1 / fieldAspect)
        {
            // landscape
            auto outputsBounds = bounds.removeFromRight(int(bounds.getHeight() / fieldAspect));
            outputsBounds.removeFromLeft(margin / 2);
            auto inputsBounds = bounds;
            inputsBounds.removeFromRight(margin / 2);

            m_inputMeteringComponent->setBounds(inputsBounds);
            m_outputFieldComponent->setBounds(outputsBounds);
        }
        else
        {
            // portrait
            auto outputBounds = bounds.removeFromBottom(int(bounds.getWidth() * fieldAspect));
            outputBounds.removeFromTop(margin / 2);
            auto inputBounds = bounds;
            inputBounds.removeFromBottom(margin / 2);

            m_inputMeteringComponent->setBounds(inputBounds);
            m_outputFieldComponent->setBounds(outputBounds);
        }
    }
    else if (m_inputMeteringComponent && m_outputMeteringComponent)
    {
        auto margin = 8;
        auto bounds = getLocalBounds().reduced(margin, margin);
        if (bounds.getAspectRatio() >= 1)
        {
            // landscape
            if (m_inputMeteringComponent && m_outputMeteringComponent)
            {
                auto ic = float(m_inputMeteringComponent->getChannelCount());
                auto oc = float(m_outputMeteringComponent->getChannelCount());

                if (0.0f != ic && 0.0f != oc)
                    m_ioRatio = ic / (ic + oc);
            }

            auto inputsBounds = bounds.removeFromLeft(int(bounds.getWidth() * m_ioRatio));
            inputsBounds.removeFromRight(margin / 2);
            auto outputsBounds = bounds;
            outputsBounds.removeFromLeft(margin / 2);

            m_inputMeteringComponent->setBounds(inputsBounds);
            m_outputMeteringComponent->setBounds(outputsBounds);
        }
        else
        {
            // portrait
            auto inputBounds = bounds.removeFromTop(bounds.getHeight() / 2);
            inputBounds.removeFromBottom(margin / 2);
            auto outputBounds = bounds;
            outputBounds.removeFromTop(margin / 2);

            m_inputMeteringComponent->setBounds(inputBounds);
            m_outputMeteringComponent->setBounds(outputBounds);
        }
    }
    else if (m_waveformComponent)
    {
        auto margin = 8;
        auto bounds = getLocalBounds().reduced(margin, margin);
        m_waveformComponent->setBounds(bounds);
    }
}

void MemaMoComponent::handleMessage(const Message& message)
{
    if (RunningStatus::Active != m_runningStatus)
    {
        m_runningStatus = RunningStatus::Active;
        resized();
    }
    
    if (auto const apm = dynamic_cast<const Mema::AnalyzerParametersMessage*>(&message))
    {
        auto sampleRate = apm->getSampleRate();
        jassert(sampleRate > 0);
        auto maximumExpectedSamplesPerBlock = apm->getMaximumExpectedSamplesPerBlock();
        jassert(maximumExpectedSamplesPerBlock > 0);

        if (m_inputDataAnalyzer)
            m_inputDataAnalyzer->initializeParameters(sampleRate, maximumExpectedSamplesPerBlock);
        if (m_outputDataAnalyzer)
            m_outputDataAnalyzer->initializeParameters(sampleRate, maximumExpectedSamplesPerBlock);
    }
    else if (auto const iom = dynamic_cast<const Mema::ReinitIOCountMessage*>(&message))
    {
        auto inputCount = iom->getInputCount();
        jassert(inputCount > 0);
        if (m_inputMeteringComponent)
            m_inputMeteringComponent->setChannelCount(inputCount);
        auto outputCount = iom->getOutputCount();
        jassert(outputCount > 0);
        if (m_outputMeteringComponent)
            m_outputMeteringComponent->setChannelCount(outputCount);

        m_currentIOCount = std::make_pair(inputCount, outputCount);

        resized();
    }
    else if (auto m = dynamic_cast<const Mema::AudioBufferMessage*>(&message))
    {
        if (m->getFlowDirection() == Mema::AudioBufferMessage::FlowDirection::Input && m_inputDataAnalyzer)
        {
            m_inputDataAnalyzer->analyzeData(m->getAudioBuffer());
        }
        else if (m->getFlowDirection() == Mema::AudioBufferMessage::FlowDirection::Output && m_outputDataAnalyzer)
        {
            m_outputDataAnalyzer->analyzeData(m->getAudioBuffer());
        }
    }
}


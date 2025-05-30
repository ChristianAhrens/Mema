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

//#include "MemaProcessorEditor/MeterbridgeComponent.h"
//#include "MemaProcessorEditor/TwoDFieldOutputComponent.h"
#include "MemaProcessor/MemaMessages.h"
#include "MemaProcessor/MemaProcessor.h"
//#include "MemaProcessor/ProcessorDataAnalyzer.h"

MemaReComponent::MemaReComponent()
    : juce::Component()
{
    //m_inputMeteringComponent = std::make_unique<Mema::MeterbridgeComponent>(Mema::MeterbridgeComponent::Direction::Horizontal);
    //addAndMakeVisible(m_inputMeteringComponent.get());
    //
    //m_inputDataAnalyzer = std::make_unique<Mema::ProcessorDataAnalyzer>();
    //m_inputDataAnalyzer->addListener(m_inputMeteringComponent.get());
    //
    //m_outputDataAnalyzer = std::make_unique<Mema::ProcessorDataAnalyzer>();

    setOutputFaderbankCtrlActive();
}

MemaReComponent::~MemaReComponent()
{
}

void MemaReComponent::setOutputFaderbankCtrlActive()
{
    auto resizeRequired = false;
    //if (!m_outputMeteringComponent)
    //{
    //    m_outputMeteringComponent = std::make_unique<Mema::MeterbridgeComponent>(Mema::MeterbridgeComponent::Direction::Horizontal);
    //    m_outputMeteringComponent->setChannelCount(m_currentIOCount.second);
    //    addAndMakeVisible(m_outputMeteringComponent.get());
    //    if (m_outputDataAnalyzer)
    //        m_outputDataAnalyzer->addListener(m_outputMeteringComponent.get());
    //    resizeRequired = true;
    //}
    //
    //if (m_outputFieldComponent)
    //{
    //    if (m_outputDataAnalyzer)
    //        m_outputDataAnalyzer->removeListener(m_outputFieldComponent.get());
    //
    //    removeChildComponent(m_outputFieldComponent.get());
    //    m_outputFieldComponent.reset();
    //    resizeRequired = true;
    //}

    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaReComponent::setOutputPanningCtrlActive(const juce::AudioChannelSet& channelConfiguration)
{
    auto resizeRequired = false;
    //if (!m_outputFieldComponent)
    //{
    //    m_outputFieldComponent = std::make_unique<Mema::TwoDFieldOutputComponent>();
    //    addAndMakeVisible(m_outputFieldComponent.get());
    //    if (m_outputDataAnalyzer)
    //        m_outputDataAnalyzer->addListener(m_outputFieldComponent.get());
    //    resizeRequired = true;
    //}
    //if (m_outputFieldComponent->setChannelConfiguration(channelConfiguration))
    //    resizeRequired = true;
    //
    //if (m_outputMeteringComponent)
    //{
    //    if (m_outputDataAnalyzer)
    //        m_outputDataAnalyzer->removeListener(m_outputMeteringComponent.get());
    //
    //    removeChildComponent(m_outputMeteringComponent.get());
    //    m_outputMeteringComponent.reset();
    //    resizeRequired = true;
    //}

    if (resizeRequired && !getLocalBounds().isEmpty())
        resized();
}

void MemaReComponent::paint(Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::widgetBackground));
}

void MemaReComponent::resized()
{
    //if (m_inputMeteringComponent && m_outputFieldComponent)
    //{
    //    auto margin = 8;
    //    auto bounds = getLocalBounds().reduced(margin, margin);
    //    auto boundsAspect = bounds.toFloat().getAspectRatio();
    //    auto fieldAspect = m_outputFieldComponent->getRequiredAspectRatio();
    //    if (boundsAspect >= 1 / fieldAspect)
    //    {
    //        // landscape
    //        auto outputsBounds = bounds.removeFromRight(int(bounds.getHeight() / fieldAspect));
    //        outputsBounds.removeFromLeft(margin / 2);
    //        auto inputsBounds = bounds;
    //        inputsBounds.removeFromRight(margin / 2);
    //
    //        m_inputMeteringComponent->setBounds(inputsBounds);
    //        m_outputFieldComponent->setBounds(outputsBounds);
    //    }
    //    else
    //    {
    //        // portrait
    //        auto outputBounds = bounds.removeFromBottom(int(bounds.getWidth() * fieldAspect));
    //        outputBounds.removeFromTop(margin / 2);
    //        auto inputBounds = bounds;
    //        inputBounds.removeFromBottom(margin / 2);
    //
    //        m_inputMeteringComponent->setBounds(inputBounds);
    //        m_outputFieldComponent->setBounds(outputBounds);
    //    }
    //}
    //else if (m_inputMeteringComponent && m_outputMeteringComponent)
    //{
    //    auto margin = 8;
    //    auto bounds = getLocalBounds().reduced(margin, margin);
    //    if (bounds.getAspectRatio() >= 1)
    //    {
    //        // landscape
    //        if (m_inputMeteringComponent && m_outputMeteringComponent)
    //        {
    //            auto ic = float(m_inputMeteringComponent->getChannelCount());
    //            auto oc = float(m_outputMeteringComponent->getChannelCount());
    //
    //            if (0.0f != ic && 0.0f != oc)
    //                m_ioRatio = ic / (ic + oc);
    //        }
    //
    //        auto inputsBounds = bounds.removeFromLeft(int(bounds.getWidth() * m_ioRatio));
    //        inputsBounds.removeFromRight(margin / 2);
    //        auto outputsBounds = bounds;
    //        outputsBounds.removeFromLeft(margin / 2);
    //
    //        m_inputMeteringComponent->setBounds(inputsBounds);
    //        m_outputMeteringComponent->setBounds(outputsBounds);
    //    }
    //    else
    //    {
    //        // portrait
    //        auto inputBounds = bounds.removeFromTop(bounds.getHeight() / 2);
    //        inputBounds.removeFromBottom(margin / 2);
    //        auto outputBounds = bounds;
    //        outputBounds.removeFromTop(margin / 2);
    //
    //        m_inputMeteringComponent->setBounds(inputBounds);
    //        m_outputMeteringComponent->setBounds(outputBounds);
    //    }
    //}
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
        jassertfalse; // we don't want analyzer parameter infos!
    }
    else if (auto m = dynamic_cast<const Mema::AudioBufferMessage*>(&message))
    {
        jassertfalse; // we don't want audio buffer infos!
    }
    else if (auto const iom = dynamic_cast<const Mema::ReinitIOCountMessage*>(&message))
    {
        auto inputCount = iom->getInputCount();
        jassert(inputCount > 0);
        auto outputCount = iom->getOutputCount();
        jassert(outputCount > 0);

        m_currentIOCount = std::make_pair(inputCount, outputCount);

        //

        resized();
    }
    else if (auto const cpm = dynamic_cast<const Mema::ControlParametersMessage*>(&message))
    {
        m_inputMuteStates = cpm->getInputMuteStates();
        jassert(m_inputMuteStates.empty());
        m_outputMuteStates = cpm->getOutputMuteStates();
        jassert(m_outputMuteStates.empty());
        m_crosspointStates = cpm->getCrosspointStates();
        jassert(m_crosspointStates.empty());

        //

        resized();
    }
}


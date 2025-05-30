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

#include "MemaClientControlComponents.h"


namespace Mema
{


MemaClientControlComponentBase::MemaClientControlComponentBase()
    : juce::Component()
{
}

MemaClientControlComponentBase::~MemaClientControlComponentBase()
{
}

void MemaClientControlComponentBase::setIOCount(const std::pair<int, int>& ioCount)
{
    m_ioCount = ioCount;
}

const std::pair<int, int>& MemaClientControlComponentBase::getIOCount()
{
    return m_ioCount;
}

void MemaClientControlComponentBase::setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates)
{
    m_inputMuteStates = inputMuteStates;
}

const std::map<std::uint16_t, bool>& MemaClientControlComponentBase::getInputMuteStates()
{
    return m_inputMuteStates;
}

void MemaClientControlComponentBase::setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates)
{
    m_outputMuteStates = outputMuteStates;
}

const std::map<std::uint16_t, bool>& MemaClientControlComponentBase::getOutputMuteStates()
{
    return m_outputMuteStates;
}

void MemaClientControlComponentBase::setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& crosspointStates)
{
    m_crosspointStates = crosspointStates;
}

const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& MemaClientControlComponentBase::getCrosspointStates()
{
    return m_crosspointStates;
}

const juce::String MemaClientControlComponentBase::getClientControlParametersAsString()
{
    auto controlParametersStr = juce::String("IO ");
    controlParametersStr << "\n" << getIOCount().first << "x" << getIOCount().second << "\n\n";

    controlParametersStr << "InputMutes: " << "\n";
    for (auto const& mutestate : getInputMuteStates())
        controlParametersStr << int(mutestate.first) << ":" << (mutestate.second ? "on" : "off") << ";";
    controlParametersStr << "\n\n";

    controlParametersStr << "OutputMutes: " << "\n";
    for (auto const& mutestate : getOutputMuteStates())
        controlParametersStr << int(mutestate.first) << ":" << (mutestate.second ? "on" : "off") << ";";
    controlParametersStr << "\n\n";

    controlParametersStr << "Crosspoints: " << "\n";
    for (auto const& crosspointstateFKV : getCrosspointStates())
    {
        auto in = int(crosspointstateFKV.first);
        for (auto const& crosspointstateSKV : crosspointstateFKV.second)
        {
            auto out = int(crosspointstateSKV.first);
            controlParametersStr << in << "." << out << ":" << (crosspointstateSKV.second.first ? "on" : "off") << "(" << crosspointstateSKV.second.second << ");";
        }
        controlParametersStr << "\n";
    }
    controlParametersStr << "\n";

    return controlParametersStr;
}


FaderbankControlComponent::FaderbankControlComponent()
    : MemaClientControlComponentBase()
{
    m_inputControlsGrid = std::make_unique<juce::Grid>();
    m_inputControlsGrid->rowGap.pixels = 2;
    m_inputControlsGrid->columnGap.pixels = 2;
    m_outputControlsGrid = std::make_unique<juce::Grid>();
    m_outputControlsGrid->rowGap.pixels = 2;
    m_outputControlsGrid->columnGap.pixels = 2;
    m_crosspointsControlsGrid = std::make_unique<juce::Grid>();
    m_crosspointsControlsGrid->rowGap.pixels = 2;
    m_crosspointsControlsGrid->columnGap.pixels = 2;
}

FaderbankControlComponent::~FaderbankControlComponent()
{
}

void FaderbankControlComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::ColourIds::backgroundColourId));

    g.setColour(getLookAndFeel().findColour(juce::TextEditor::ColourIds::textColourId));
    g.drawFittedText("Faderbank config:\n\n" + getClientControlParametersAsString(), getLocalBounds().reduced(35), juce::Justification::topLeft, 22);
}

void FaderbankControlComponent::resized()
{
    auto ctrlsSize = 80;
    auto bounds = getLocalBounds();
    auto inputControlBounds = bounds.removeFromTop(ctrlsSize);
    inputControlBounds.removeFromLeft(ctrlsSize);
    auto outputControlBounds = bounds.removeFromLeft(ctrlsSize);

    if (m_inputControlsGrid)
        m_inputControlsGrid->performLayout(inputControlBounds);
    if (m_outputControlsGrid)
        m_outputControlsGrid->performLayout(outputControlBounds);
}

void FaderbankControlComponent::setIOCount(const std::pair<int, int>& ioCount)
{
    MemaClientControlComponentBase::setIOCount(ioCount);

    m_inputControlsGrid->templateColumns.clear();
    m_inputControlsGrid->items.resize(ioCount.first);
    m_inputMuteButtons.resize(ioCount.first);
    m_inputSelectButtons.resize(ioCount.first);
    for (auto i = 0; i < ioCount.first; i++)
    {
        m_inputControlsGrid->templateColumns.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
        if (nullptr == m_inputControlsGrid->items[i].associatedComponent)
        {
            m_inputMuteButtons.at(i) = std::make_unique<juce::TextButton>("M");
            addAndMakeVisible(m_inputMuteButtons.at(i).get());
            m_inputControlsGrid->items[i] = juce::GridItem(m_inputMuteButtons.at(i).get());
        }
        if (nullptr == m_inputControlsGrid->items[i + ioCount.first].associatedComponent)
        {
            m_inputSelectButtons.at(i) = std::make_unique<juce::TextButton>(juce::String(i+1));
            addAndMakeVisible(m_inputSelectButtons.at(i).get());
            m_inputControlsGrid->items[i + ioCount.first] = juce::GridItem(m_inputSelectButtons.at(i).get());
        }
    }
    m_inputControlsGrid->templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    //m_inputControlsGrid->autoColumns = juce::Grid::TrackInfo(juce::Grid::Fr(10));
    //m_inputControlsGrid->autoRows = juce::Grid::TrackInfo(juce::Grid::Fr(10));
    //m_inputControlsGrid->autoFlow = juce::Grid::AutoFlow::row;

    m_outputControlsGrid->templateRows.clear();
    m_outputControlsGrid->items.resize(ioCount.second);
    m_outputMuteButtons.resize(ioCount.second);
    m_outputSelectButtons.resize(ioCount.second);
    for (auto i = 0; i < ioCount.second; i++)
    {
        m_outputControlsGrid->templateRows.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
        if (nullptr == m_outputControlsGrid->items[i].associatedComponent)
        {
            m_outputMuteButtons.at(i) = std::make_unique<juce::TextButton>("M");
            addAndMakeVisible(m_outputMuteButtons.at(i).get());
            m_outputControlsGrid->items[i] = juce::GridItem(m_outputMuteButtons.at(i).get());
        }
        if (nullptr == m_outputControlsGrid->items[i + ioCount.second].associatedComponent)
        {
            m_outputSelectButtons.at(i) = std::make_unique<juce::TextButton>(juce::String(i + 1));
            addAndMakeVisible(m_outputSelectButtons.at(i).get());
            m_outputControlsGrid->items[i + ioCount.second] = juce::GridItem(m_outputSelectButtons.at(i).get());
        }
    }
    m_outputControlsGrid->templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    //m_outputControlsGrid->autoColumns = juce::Grid::TrackInfo(juce::Grid::Fr(10));
    //m_outputControlsGrid->autoRows = juce::Grid::TrackInfo(juce::Grid::Fr(10));
    //m_outputControlsGrid->autoFlow = juce::Grid::AutoFlow::column;

    resized();
}

void FaderbankControlComponent::setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates)
{
    MemaClientControlComponentBase::setInputMuteStates(inputMuteStates);

    //
}

void FaderbankControlComponent::setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates)
{
    MemaClientControlComponentBase::setOutputMuteStates(outputMuteStates);

    //
}

void FaderbankControlComponent::setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& crosspointStates)
{
    MemaClientControlComponentBase::setCrosspointStates(crosspointStates);

    //
}


PanningControlComponent::PanningControlComponent()
    : MemaClientControlComponentBase()
{
}

PanningControlComponent::~PanningControlComponent()
{
}

void PanningControlComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::ColourIds::backgroundColourId));

    g.setColour(getLookAndFeel().findColour(juce::TextEditor::ColourIds::textColourId));
    g.drawFittedText("Panning control not yet implemented.\n(Panning config is " + m_channelConfiguration.getSpeakerArrangementAsString() + ")\n\n" + getClientControlParametersAsString(), getLocalBounds().reduced(35), juce::Justification::topLeft, 24);
}

void PanningControlComponent::resized()
{
}

void PanningControlComponent::setChannelConfig(const juce::AudioChannelSet& channelConfiguration)
{
    m_channelConfiguration = channelConfiguration;
}

const juce::AudioChannelSet& PanningControlComponent::getChannelConfig()
{
    return m_channelConfiguration;
}


} // namespace Mema

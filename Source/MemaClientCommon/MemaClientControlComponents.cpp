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

    /*//test
    m_inputControlsGrid->templateRows = { 
        juce::Grid::TrackInfo(juce::Grid::Fr(1)), 
        juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_inputControlsGrid->templateColumns = {
        juce::Grid::TrackInfo(juce::Grid::Fr(1)), 
        juce::Grid::TrackInfo(juce::Grid::Fr(1)),
        juce::Grid::TrackInfo(juce::Grid::Fr(1)),
        juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

    m_inputMuteButtons.resize(4);
    m_inputMuteButtons.at(0) = std::make_unique<juce::TextButton>("MI1");
    addAndMakeVisible(m_inputMuteButtons.at(0).get());
    m_inputMuteButtons.at(1) = std::make_unique<juce::TextButton>("MI2");
    addAndMakeVisible(m_inputMuteButtons.at(1).get());
    m_inputMuteButtons.at(2) = std::make_unique<juce::TextButton>("MI3");
    addAndMakeVisible(m_inputMuteButtons.at(2).get());
    m_inputMuteButtons.at(3) = std::make_unique<juce::TextButton>("MI4");
    addAndMakeVisible(m_inputMuteButtons.at(3).get());

    m_inputSelectButtons.resize(4);
    m_inputSelectButtons.at(0) = std::make_unique<juce::TextButton>("I" + juce::String(1));
    addAndMakeVisible(m_inputSelectButtons.at(0).get());
    m_inputSelectButtons.at(1) = std::make_unique<juce::TextButton>("I" + juce::String(2));
    addAndMakeVisible(m_inputSelectButtons.at(1).get());
    m_inputSelectButtons.at(2) = std::make_unique<juce::TextButton>("I" + juce::String(3));
    addAndMakeVisible(m_inputSelectButtons.at(2).get());
    m_inputSelectButtons.at(3) = std::make_unique<juce::TextButton>("I" + juce::String(4));
    addAndMakeVisible(m_inputSelectButtons.at(3).get());

    m_inputControlsGrid->items = { 
        juce::GridItem(m_inputMuteButtons.at(0).get()),
        juce::GridItem(m_inputMuteButtons.at(1).get()),
        juce::GridItem(m_inputMuteButtons.at(2).get()),
        juce::GridItem(m_inputMuteButtons.at(3).get()),
        juce::GridItem(m_inputSelectButtons.at(0).get()),
        juce::GridItem(m_inputSelectButtons.at(1).get()),
        juce::GridItem(m_inputSelectButtons.at(2).get()),
        juce::GridItem(m_inputSelectButtons.at(3).get()) };

    //m_inputControlsGrid->autoColumns = juce::Grid::TrackInfo(juce::Grid::Fr(10));
    //m_inputControlsGrid->autoRows = juce::Grid::TrackInfo(juce::Grid::Fr(10));
    //m_inputControlsGrid->autoFlow = juce::Grid::AutoFlow::row;

    m_outputControlsGrid->templateRows = {
        juce::Grid::TrackInfo(juce::Grid::Fr(1)),
        juce::Grid::TrackInfo(juce::Grid::Fr(1)),
        juce::Grid::TrackInfo(juce::Grid::Fr(1)),
        juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_outputControlsGrid->templateColumns = { 
        juce::Grid::TrackInfo(juce::Grid::Fr(1)), 
        juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

    m_outputMuteButtons.resize(4);
    m_outputMuteButtons.at(0) = std::make_unique<juce::TextButton>("MO1");
    addAndMakeVisible(m_outputMuteButtons.at(0).get());
    m_outputMuteButtons.at(1) = std::make_unique<juce::TextButton>("MO2");
    addAndMakeVisible(m_outputMuteButtons.at(1).get());
    m_outputMuteButtons.at(2) = std::make_unique<juce::TextButton>("MO3");
    addAndMakeVisible(m_outputMuteButtons.at(2).get());
    m_outputMuteButtons.at(3) = std::make_unique<juce::TextButton>("MO4");
    addAndMakeVisible(m_outputMuteButtons.at(3).get());

    m_outputSelectButtons.resize(4);
    m_outputSelectButtons.at(0) = std::make_unique<juce::TextButton>("O" + juce::String(1));
    addAndMakeVisible(m_outputSelectButtons.at(0).get());
    m_outputSelectButtons.at(1) = std::make_unique<juce::TextButton>("O" + juce::String(2));
    addAndMakeVisible(m_outputSelectButtons.at(1).get());
    m_outputSelectButtons.at(2) = std::make_unique<juce::TextButton>("O" + juce::String(3));
    addAndMakeVisible(m_outputSelectButtons.at(2).get());
    m_outputSelectButtons.at(3) = std::make_unique<juce::TextButton>("O" + juce::String(4));
    addAndMakeVisible(m_outputSelectButtons.at(3).get());
            
    m_outputControlsGrid->items = {
        juce::GridItem(m_outputMuteButtons.at(0).get()),
        juce::GridItem(m_outputMuteButtons.at(1).get()),
        juce::GridItem(m_outputMuteButtons.at(2).get()),
        juce::GridItem(m_outputMuteButtons.at(3).get()),
        juce::GridItem(m_outputSelectButtons.at(0).get()),
        juce::GridItem(m_outputSelectButtons.at(1).get()),
        juce::GridItem(m_outputSelectButtons.at(2).get()),
        juce::GridItem(m_outputSelectButtons.at(3).get()) };

    //m_outputControlsGrid->autoColumns = juce::Grid::TrackInfo(juce::Grid::Fr(10));
    //m_outputControlsGrid->autoRows = juce::Grid::TrackInfo(juce::Grid::Fr(10));
    //m_outputControlsGrid->autoFlow = juce::Grid::AutoFlow::column;
    */
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

    // input controls
    auto templateColums = juce::Array<juce::Grid::TrackInfo>();
    for (auto i = 0; i < ioCount.first; i++)
        templateColums.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
    
    m_inputControlsGrid->items.clear();// .resize(ioCount.first);
    m_inputMuteButtons.resize(ioCount.first);
    m_inputSelectButtons.resize(ioCount.first);
    m_inputControlsGrid->templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_inputControlsGrid->templateColumns = templateColums;

    for (auto in = 0; in < ioCount.first; in++)
    {
        if (nullptr == m_inputControlsGrid->items[in + ioCount.first].associatedComponent || nullptr == m_inputSelectButtons.at(in))
        {
            m_inputSelectButtons.at(in) = std::make_unique<juce::TextButton>(juce::String(in + 1));
            addAndMakeVisible(m_inputSelectButtons.at(in).get());
            m_inputControlsGrid->items.add(juce::GridItem(m_inputSelectButtons.at(in).get()));
        }
    }
    for (auto in = 0; in < ioCount.first; in++)
    {
        if (nullptr == m_inputControlsGrid->items[in].associatedComponent || nullptr == m_inputMuteButtons.at(in))
        {
            m_inputMuteButtons.at(in) = std::make_unique<juce::TextButton>("M");
            m_inputMuteButtons.at(in)->setClickingTogglesState(true);
            m_inputMuteButtons.at(in)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
            addAndMakeVisible(m_inputMuteButtons.at(in).get());
            m_inputControlsGrid->items.add(juce::GridItem(m_inputMuteButtons.at(in).get()));
        }
    }

    // output controls
    auto templateRows = juce::Array<juce::Grid::TrackInfo>();
    for (auto i = 0; i < ioCount.second; i++)
        templateRows.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));

    m_outputControlsGrid->items.clear();// .resize(ioCount.second);
    m_outputMuteButtons.resize(ioCount.second);
    m_outputSelectButtons.resize(ioCount.second);
    m_outputControlsGrid->templateRows = templateRows;
    m_outputControlsGrid->templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

    for (auto out = 0; out < ioCount.second; out++)
    {
        if (nullptr == m_outputControlsGrid->items[out + ioCount.second].associatedComponent || nullptr == m_outputSelectButtons.at(out))
        {
            m_outputSelectButtons.at(out) = std::make_unique<juce::TextButton>(juce::String(out + 1));
            addAndMakeVisible(m_outputSelectButtons.at(out).get());
            m_outputControlsGrid->items.add(juce::GridItem(m_outputSelectButtons.at(out).get()));
        }
        if (nullptr == m_outputControlsGrid->items[out].associatedComponent || nullptr == m_outputMuteButtons.at(out))
        {
            m_outputMuteButtons.at(out) = std::make_unique<juce::TextButton>("M");
            m_outputMuteButtons.at(out)->setClickingTogglesState(true);
            m_outputMuteButtons.at(out)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
            addAndMakeVisible(m_outputMuteButtons.at(out).get());
            m_outputControlsGrid->items.add(juce::GridItem(m_outputMuteButtons.at(out).get()));
        }
    }

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

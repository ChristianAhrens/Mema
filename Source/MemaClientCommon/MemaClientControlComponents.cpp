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

#include <CustomLookAndFeel.h>


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
    m_inputControlsGrid->rowGap.pixels = 3;
    m_inputControlsGrid->columnGap.pixels = 3;
    m_outputControlsGrid = std::make_unique<juce::Grid>();
    m_outputControlsGrid->rowGap.pixels = 3;
    m_outputControlsGrid->columnGap.pixels = 3;
    m_crosspointsControlsGrid = std::make_unique<juce::Grid>();
    m_crosspointsControlsGrid->rowGap.pixels = 5;
    m_crosspointsControlsGrid->columnGap.pixels = 5;
}

FaderbankControlComponent::~FaderbankControlComponent()
{
}

void FaderbankControlComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::ColourIds::backgroundColourId));

    //g.setColour(getLookAndFeel().findColour(juce::TextEditor::ColourIds::textColourId));
    //g.drawFittedText("Faderbank config:\n\n" + getClientControlParametersAsString(), getLocalBounds().reduced(35), juce::Justification::topLeft, 22);
}

void FaderbankControlComponent::resized()
{
    auto ctrlsSize = 75;
    auto bounds = getLocalBounds();
    auto inputControlBounds = bounds.removeFromTop(ctrlsSize);
    inputControlBounds.removeFromLeft(ctrlsSize);
    auto outputControlBounds = bounds.removeFromLeft(ctrlsSize);
    auto crosspointControlBounds = bounds;

    if (m_inputControlsGrid)
        m_inputControlsGrid->performLayout(inputControlBounds);
    if (m_outputControlsGrid)
        m_outputControlsGrid->performLayout(outputControlBounds);
    if (m_crosspointsControlsGrid)
        m_crosspointsControlsGrid->performLayout(crosspointControlBounds);
}

void FaderbankControlComponent::lookAndFeelChanged()
{
    auto ioCount = getIOCount();
    for (auto in = 0; in < ioCount.first; in++)
    {
        if (nullptr != m_inputSelectButtons.at(in))
            m_inputSelectButtons.at(in)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
        if (nullptr != m_crosspointGainSliders.at(in))
            m_crosspointGainSliders.at(in)->setColour(juce::Slider::ColourIds::thumbColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    }
    for (auto out = 0; out < ioCount.second; out++)
    {
        if (nullptr != m_outputSelectButtons.at(out))
            m_outputSelectButtons.at(out)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    }
}

void FaderbankControlComponent::setIOCount(const std::pair<int, int>& ioCount)
{
    MemaClientControlComponentBase::setIOCount(ioCount);

    // input controls
    auto templateColums = juce::Array<juce::Grid::TrackInfo>();
    for (auto i = 0; i < ioCount.first; i++)
        templateColums.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
    
    m_inputControlsGrid->items.clear();
    m_inputMuteButtons.resize(ioCount.first);
    m_inputSelectButtons.resize(ioCount.first);
    m_inputControlsGrid->templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_inputControlsGrid->templateColumns = templateColums;

    for (auto in = 0; in < ioCount.first; in++)
    {
        if (nullptr == m_inputControlsGrid->items[in + ioCount.first].associatedComponent || nullptr == m_inputSelectButtons.at(in))
        {
            m_inputSelectButtons.at(in) = std::make_unique<juce::TextButton>(juce::String(in + 1));
            m_inputSelectButtons.at(in)->setClickingTogglesState(true);
            m_inputSelectButtons.at(in)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
            m_inputSelectButtons.at(in)->onClick = [this, in] { selectIOChannel(ControlDirection::Input, in); };
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

    m_outputControlsGrid->items.clear();
    m_outputMuteButtons.resize(ioCount.second);
    m_outputSelectButtons.resize(ioCount.second);
    m_outputControlsGrid->templateRows = templateRows;
    m_outputControlsGrid->templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

    for (auto out = 0; out < ioCount.second; out++)
    {
        if (nullptr == m_outputControlsGrid->items[out + ioCount.second].associatedComponent || nullptr == m_outputSelectButtons.at(out))
        {
            m_outputSelectButtons.at(out) = std::make_unique<juce::TextButton>(juce::String(out + 1));
            m_outputSelectButtons.at(out)->setClickingTogglesState(true);
            m_outputSelectButtons.at(out)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
            m_outputSelectButtons.at(out)->onClick = [this, out] { selectIOChannel(ControlDirection::Output, out); };
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

    // crosspoint controls
    templateColums.clear();
    for (auto i = 0; i < ioCount.first; i++)
        templateColums.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));

    m_crosspointsControlsGrid->items.clear();
    m_crosspointGainSliders.resize(ioCount.first);
    m_crosspointsControlsGrid->templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_crosspointsControlsGrid->templateColumns = templateColums;

    for (auto in = 0; in < ioCount.first; in++)
    {
        if (nullptr == m_crosspointsControlsGrid->items[in + ioCount.first].associatedComponent || nullptr == m_crosspointGainSliders.at(in))
        {
            m_crosspointGainSliders.at(in) = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::TextBoxAbove);
            m_crosspointGainSliders.at(in)->setColour(juce::Slider::ColourIds::thumbColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
            addAndMakeVisible(m_crosspointGainSliders.at(in).get());
            m_crosspointsControlsGrid->items.add(juce::GridItem(m_crosspointGainSliders.at(in).get()));
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

void FaderbankControlComponent::selectIOChannel(const ControlDirection& direction, int channel)
{
    m_currentIOChannel = std::make_pair(direction, channel);

    auto ioCount = getIOCount();
    for (auto in = 0; in < ioCount.first; in++)
    {
        if (nullptr != m_inputSelectButtons.at(in))
        {
            auto state = (ControlDirection::Input == direction && channel == in);
            m_inputSelectButtons.at(in)->setToggleState(state, juce::dontSendNotification);
        }
    }
    for (auto out = 0; out < ioCount.second; out++)
    {
        if (nullptr != m_outputSelectButtons.at(out))
        {
            auto state = (ControlDirection::Output == direction && channel == out);
            m_outputSelectButtons.at(out)->setToggleState(state, juce::dontSendNotification);
        }
    }
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

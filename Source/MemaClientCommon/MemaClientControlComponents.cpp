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
    m_inputControlsGrid->setGap(3_px);
    m_outputControlsGrid = std::make_unique<juce::Grid>();
    m_outputControlsGrid->setGap(3_px);
    m_crosspointsControlsGrid = std::make_unique<juce::Grid>();
    m_crosspointsControlsGrid->setGap(5_px);
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
    if (m_crosspointsControlsGrid && m_currentIOChannel.first != ControlDirection::None)
        m_crosspointsControlsGrid->performLayout(crosspointControlBounds);
}

void FaderbankControlComponent::lookAndFeelChanged()
{
    auto ioCount = getIOCount();
    for (auto in = 0; in < ioCount.first; in++)
    {
        if (nullptr != m_inputSelectButtons.at(in))
            m_inputSelectButtons.at(in)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
        if (ControlDirection::Output == m_currentIOChannel.first && m_crosspointGainSliders.size() > in && nullptr != m_crosspointGainSliders.at(in))
            m_crosspointGainSliders.at(in)->setColour(juce::Slider::ColourIds::thumbColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    }
    for (auto out = 0; out < ioCount.second; out++)
    {
        if (nullptr != m_outputSelectButtons.at(out))
            m_outputSelectButtons.at(out)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
        if (ControlDirection::Input == m_currentIOChannel.first && m_crosspointGainSliders.size() > out && nullptr != m_crosspointGainSliders.at(out))
            m_crosspointGainSliders.at(out)->setColour(juce::Slider::ColourIds::thumbColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    }
}

void FaderbankControlComponent::setIOCount(const std::pair<int, int>& ioCount)
{
    MemaClientControlComponentBase::setIOCount(ioCount);

    rebuildControls();
}

void FaderbankControlComponent::rebuildControls()
{
    auto ioCount = getIOCount();

    // input controls
    auto templateColums = juce::Array<juce::Grid::TrackInfo>();
    for (auto in = 0; in < ioCount.first; in++)
        templateColums.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));
    
    m_inputMuteButtons.resize(ioCount.first);
    m_inputSelectButtons.resize(ioCount.first);
    m_inputControlsGrid->items.clear();
    m_inputControlsGrid->templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_inputControlsGrid->templateColumns = templateColums;

    for (auto in = 0; in < ioCount.first; in++)
    {
        m_inputSelectButtons.at(in) = std::make_unique<juce::TextButton>("In " + juce::String(in + 1));
        m_inputSelectButtons.at(in)->setClickingTogglesState(true);
        m_inputSelectButtons.at(in)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
        m_inputSelectButtons.at(in)->onClick = [this, in] { selectIOChannel(ControlDirection::Input, in); };
        addAndMakeVisible(m_inputSelectButtons.at(in).get());
        m_inputControlsGrid->items.add(juce::GridItem(m_inputSelectButtons.at(in).get()));
    }
    for (auto in = 0; in < ioCount.first; in++)
    {
        m_inputMuteButtons.at(in) = std::make_unique<juce::TextButton>("M");
        m_inputMuteButtons.at(in)->setClickingTogglesState(true);
        m_inputMuteButtons.at(in)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
        m_inputMuteButtons.at(in)->onClick = [this, in] {
            auto inputMuteStates = getInputMuteStates();
            inputMuteStates[in] = m_inputMuteButtons.at(in)->getToggleState();
            MemaClientControlComponentBase::setInputMuteStates(inputMuteStates);
            if (onInputMutesChanged)
                onInputMutesChanged(inputMuteStates);
        };
        addAndMakeVisible(m_inputMuteButtons.at(in).get());
        m_inputControlsGrid->items.add(juce::GridItem(m_inputMuteButtons.at(in).get()));
    }

    // output controls
    auto templateRows = juce::Array<juce::Grid::TrackInfo>();
    for (auto out = 0; out < ioCount.second; out++)
        templateRows.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));

    m_outputMuteButtons.resize(ioCount.second);
    m_outputSelectButtons.resize(ioCount.second);
    m_outputControlsGrid->items.clear();
    m_outputControlsGrid->templateRows = templateRows;
    m_outputControlsGrid->templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

    for (auto out = 0; out < ioCount.second; out++)
    {
        m_outputSelectButtons.at(out) = std::make_unique<juce::TextButton>("Out " + juce::String(out + 1));
        m_outputSelectButtons.at(out)->setClickingTogglesState(true);
        m_outputSelectButtons.at(out)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
        m_outputSelectButtons.at(out)->onClick = [this, out] { selectIOChannel(ControlDirection::Output, out); };
        addAndMakeVisible(m_outputSelectButtons.at(out).get());
        m_outputControlsGrid->items.add(juce::GridItem(m_outputSelectButtons.at(out).get()));

        m_outputMuteButtons.at(out) = std::make_unique<juce::TextButton>("M");
        m_outputMuteButtons.at(out)->setClickingTogglesState(true);
        m_outputMuteButtons.at(out)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
        m_outputMuteButtons.at(out)->onClick = [this, out] {
            auto outputMuteStates = getInputMuteStates();
            outputMuteStates[out] = m_outputMuteButtons.at(out)->getToggleState();
            MemaClientControlComponentBase::setOutputMuteStates(outputMuteStates);
            if (onOutputMutesChanged)
                onOutputMutesChanged(outputMuteStates);
        };
        addAndMakeVisible(m_outputMuteButtons.at(out).get());
        m_outputControlsGrid->items.add(juce::GridItem(m_outputMuteButtons.at(out).get()));
    }

    // crosspoint controls
    if (ControlDirection::Output == m_currentIOChannel.first)
    {
        templateColums.clear();
        for (auto in = 0; in < ioCount.first; in++)
            templateColums.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));

        m_crosspointGainSliders.resize(ioCount.first);
        m_crosspointsControlsGrid->items.clear();
        m_crosspointsControlsGrid->templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        m_crosspointsControlsGrid->templateColumns = templateColums;

        for (auto in = 0; in < ioCount.first; in++)
        {
            m_crosspointGainSliders.at(in) = std::make_unique<juce::Slider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
            m_crosspointGainSliders.at(in)->setColour(juce::Slider::ColourIds::thumbColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
            m_crosspointGainSliders.at(in)->onValueChange = [this, in] {
                auto crosspointStates = getCrosspointStates();
                auto faderValue = m_crosspointGainSliders.at(in)->getValue();
                auto faderState = (faderValue != 0.0);
                crosspointStates[in][m_currentIOChannel.second] = std::make_pair(faderState, float(faderValue));
                MemaClientControlComponentBase::setCrosspointStates(crosspointStates);
                if (onCrosspointStatesChanged)
                    onCrosspointStatesChanged(crosspointStates);
            };
            addAndMakeVisible(m_crosspointGainSliders.at(in).get());
            m_crosspointsControlsGrid->items.add(juce::GridItem(m_crosspointGainSliders.at(in).get()));
        }
    }
    else if (ControlDirection::Input == m_currentIOChannel.first)
    {
        templateRows.clear();
        for (auto out = 0; out < ioCount.second; out++)
            templateRows.add(juce::Grid::TrackInfo(juce::Grid::Fr(1)));

        m_crosspointGainSliders.resize(ioCount.second);
        m_crosspointsControlsGrid->items.clear();
        m_crosspointsControlsGrid->templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        m_crosspointsControlsGrid->templateRows = templateRows;

        for (auto out = 0; out < ioCount.second; out++)
        {
            m_crosspointGainSliders.at(out) = std::make_unique<juce::Slider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
            m_crosspointGainSliders.at(out)->setColour(juce::Slider::ColourIds::thumbColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
            m_crosspointGainSliders.at(out)->onValueChange = [this, out] {
                auto crosspointStates = getCrosspointStates();
                auto faderValue = m_crosspointGainSliders.at(out)->getValue();
                auto faderState = (faderValue != 0.0);
                crosspointStates[m_currentIOChannel.second][out] = std::make_pair(faderState, float(faderValue));
                MemaClientControlComponentBase::setCrosspointStates(crosspointStates);
                if (onCrosspointStatesChanged)
                    onCrosspointStatesChanged(crosspointStates);
            }; 
            addAndMakeVisible(m_crosspointGainSliders.at(out).get());
            m_crosspointsControlsGrid->items.add(juce::GridItem(m_crosspointGainSliders.at(out).get()));
        }
    }

    resized();
}

void FaderbankControlComponent::setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates)
{
    MemaClientControlComponentBase::setInputMuteStates(inputMuteStates);

    for (auto const& inputMuteStateKV : inputMuteStates)
    {
        auto& in = inputMuteStateKV.first;
        auto& state = inputMuteStateKV.second;
        if (m_inputMuteButtons.size() > in && nullptr != m_inputMuteButtons.at(in))
            m_inputMuteButtons.at(in)->setToggleState(state, juce::dontSendNotification);
    }
}

void FaderbankControlComponent::setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates)
{
    MemaClientControlComponentBase::setOutputMuteStates(outputMuteStates);

    for (auto const& outputMuteStateKV : outputMuteStates)
    {
        auto& out = outputMuteStateKV.first;
        auto& state = outputMuteStateKV.second;
        if (m_outputMuteButtons.size() > out && nullptr != m_outputMuteButtons.at(out))
            m_outputMuteButtons.at(out)->setToggleState(state, juce::dontSendNotification);
    }
}

void FaderbankControlComponent::setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>>& crosspointStates)
{
    MemaClientControlComponentBase::setCrosspointStates(crosspointStates);

    updateCrosspointFaderValues();
}

void FaderbankControlComponent::selectIOChannel(const ControlDirection& direction, int channel)
{
    auto oldDirection = m_currentIOChannel.first;
    m_currentIOChannel = std::make_pair(direction, channel);
    if (oldDirection != direction)
        rebuildControls();

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

void FaderbankControlComponent::updateCrosspointFaderValues()
{
    auto& crosspointStates = getCrosspointStates();
    for (auto const& crosspointStateIKV : crosspointStates)
    {
        auto& in = crosspointStateIKV.first;
        for (auto const& crosspointStateIOKV : crosspointStateIKV.second)
        {
            auto& out = crosspointStateIOKV.first;
            auto& state = crosspointStateIOKV.second;

            if (ControlDirection::Input == m_currentIOChannel.first && in == m_currentIOChannel.second && m_crosspointGainSliders.size() < out)
                m_crosspointGainSliders.at(out)->setValue((state.first ? state.second : 0.0), juce::dontSendNotification);
            if (ControlDirection::Output == m_currentIOChannel.first && out == m_currentIOChannel.second && m_crosspointGainSliders.size() < in)
                m_crosspointGainSliders.at(in)->setValue((state.first ? state.second : 0.0), juce::dontSendNotification);
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

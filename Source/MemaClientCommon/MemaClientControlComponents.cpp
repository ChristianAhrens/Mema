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

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

#include "InputControlComponent.h"

#include "AbstractAudioVisualizer.h"
#include "MeterbridgeComponent.h"

namespace Mema
{

//==============================================================================
InputControlComponent::InputControlComponent()
    : AbstractAudioVisualizer(), MemaInputCommander()
{
    setUsesValuesInDB(true);

    m_inputLevels = std::make_unique<MeterbridgeComponent>(MeterbridgeComponent::Direction::Horizontal);
    addAndMakeVisible(m_inputLevels.get());
}

InputControlComponent::~InputControlComponent()
{

}

void InputControlComponent::resized()
{
    auto bounds = getLocalBounds();

    if (m_inputLevels)
        m_inputLevels->setBounds(bounds
            .removeFromTop(bounds.getHeight() - int(s_channelSize + s_channelGap))
            .removeFromLeft(int(m_inputMutes.size() * (s_channelSize + s_channelGap))));

    if (!m_inputMutes.empty())
    {
        auto muteWidth = int(s_channelSize + s_channelGap);
        for (auto const& inputMuteKV : m_inputMutes)
        {
            inputMuteKV.second->setBounds(bounds.removeFromLeft(muteWidth).reduced(2));
        }
    }

    AbstractAudioVisualizer::resized();
}

void InputControlComponent::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    // Paint whatever the base class does ontop of the solid fill
    AbstractAudioVisualizer::paint(g);
}

void InputControlComponent::setInputMute(std::uint16_t channel, bool muteState, int /*userId*/)
{
    if (m_inputMutes.count(channel) != 1)
        return;

    if (m_inputMutes.at(channel))
        m_inputMutes.at(channel)->setToggleState(muteState, juce::dontSendNotification);
}

void InputControlComponent::processingDataChanged(AbstractProcessorData *data)
{
    if(!data)
        return;
    
    switch(data->GetDataType())
    {
        case AbstractProcessorData::Level:
            m_levelData = *(dynamic_cast<ProcessorLevelData*>(data));
            notifyChanges();
            break;
        case AbstractProcessorData::AudioSignal:
        case AbstractProcessorData::Spectrum:
        case AbstractProcessorData::Invalid:
        default:
            break;
    }
}

void InputControlComponent::processChanges()
{
    if (m_inputLevels)
        m_inputLevels->processingDataChanged(&m_levelData);

    AbstractAudioVisualizer::processChanges();
}

void InputControlComponent::setChannelCount(std::uint16_t channelCount)
{
    if (m_channelCount != channelCount)
    {
        m_channelCount = channelCount;
        DBG(__FUNCTION__ << " " << int(channelCount));

        m_inputLevels->setChannelCount(channelCount);

        auto channelsToRemove = std::vector<std::uint16_t>();
        for (auto const& inputMuteKV : m_inputMutes)
        {
            if (inputMuteKV.first > channelCount)
                channelsToRemove.push_back(inputMuteKV.first);
        }
        for (auto const& channel : channelsToRemove)
        {
            removeChildComponent(m_inputMutes.at(channel).get());
            auto iter = std::find_if(m_inputMutes.begin(), m_inputMutes.end(), [=](const auto& inputMuteKV) { return inputMuteKV.first == channel; });
            if (iter != m_inputMutes.end())
                m_inputMutes.erase(iter);
        }

        for (std::uint16_t channel = 1; channel <= channelCount; channel++)
        {
            if (m_inputMutes.count(channel) == 0)
            {
                m_inputMutes[channel] = std::make_unique<juce::TextButton>("M", "Mute");
                m_inputMutes.at(channel)->setClickingTogglesState(true);
                m_inputMutes.at(channel)->addListener(this);
                m_inputMutes.at(channel)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
                addAndMakeVisible(m_inputMutes.at(channel).get());
            }
        }

        if (onBoundsRequirementChange)
            onBoundsRequirementChange();

        resized();
    }
}

void InputControlComponent::buttonClicked(juce::Button* button)
{
    auto iter = std::find_if(m_inputMutes.begin(), m_inputMutes.end(), [=](const auto& inputMuteKV) { return inputMuteKV.second.get() == button; });
    if (iter != m_inputMutes.end() && nullptr != iter->second)
        inputMuteChange(iter->first, iter->second->getToggleState(), this);
}

juce::Rectangle<int> InputControlComponent::getRequiredSize()
{
    return { m_channelCount * int(s_channelSize + s_channelGap), 60 };
}


}

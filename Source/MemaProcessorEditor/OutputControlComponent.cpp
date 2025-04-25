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

#include "OutputControlComponent.h"

#include "AbstractAudioVisualizer.h"
#include "MeterbridgeComponent.h"

namespace Mema
{

//==============================================================================
OutputControlComponent::OutputControlComponent()
    : AbstractAudioVisualizer(), MemaOutputCommander()
{
    setUsesValuesInDB(true);

    m_outputLevels = std::make_unique<MeterbridgeComponent>(MeterbridgeComponent::Direction::Vertical);
    addAndMakeVisible(m_outputLevels.get());
}

OutputControlComponent::~OutputControlComponent()
{

}

void OutputControlComponent::resized()
{
    auto bounds = getLocalBounds();

    if (m_outputLevels)
        m_outputLevels->setBounds(bounds
            .removeFromRight(bounds.getWidth() - int(s_channelSize + s_channelGap))
            .removeFromTop(int(m_outputMutes.size() * (s_channelSize + s_channelGap))));

    if (!m_outputMutes.empty())
    {
        auto muteHeight = int(s_channelSize + s_channelGap);
        for (auto const& outputMuteKV : m_outputMutes)
        {
            outputMuteKV.second->setBounds(bounds.removeFromTop(muteHeight).reduced(2));
        }
    }

    AbstractAudioVisualizer::resized();
}

void OutputControlComponent::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

    // Paint whatever the base class does ontop of the solid fill
    AbstractAudioVisualizer::paint(g);
}

void OutputControlComponent::setOutputMute(unsigned int channel, bool muteState)
{
    if (m_outputMutes.count(channel) != 1)
        return;
    
    if (m_outputMutes.at(channel))
        m_outputMutes.at(channel)->setToggleState(muteState, juce::dontSendNotification);
}

void OutputControlComponent::processingDataChanged(AbstractProcessorData *data)
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

void OutputControlComponent::processChanges()
{

    if (m_outputLevels)
        m_outputLevels->processingDataChanged(&m_levelData);

    AbstractAudioVisualizer::processChanges();
}

void OutputControlComponent::setChannelCount(int channelCount)
{
    if (m_channelCount != channelCount)
    {
        m_channelCount = channelCount;
        DBG(__FUNCTION__ << " " << channelCount);

        m_outputLevels->setChannelCount(channelCount);

        auto channelsToRemove = std::vector<int>();
        for (auto const& outputMuteKV : m_outputMutes)
        {
            if (outputMuteKV.first > channelCount)
                channelsToRemove.push_back(outputMuteKV.first);
        }
        for (auto const& channel : channelsToRemove)
        {
            removeChildComponent(m_outputMutes.at(channel).get());
            auto iter = std::find_if(m_outputMutes.begin(), m_outputMutes.end(), [=](const auto& outputMuteKV) { return outputMuteKV.first == channel; });
            if (iter != m_outputMutes.end())
                m_outputMutes.erase(iter);
        }

        for (int channel = 1; channel <= channelCount; channel++)
        {
            if (m_outputMutes.count(channel) == 0)
            {
                m_outputMutes[channel] =std::make_unique<juce::TextButton>("M", "Mute");
                m_outputMutes.at(channel)->setClickingTogglesState(true);
                m_outputMutes.at(channel)->addListener(this);
                m_outputMutes.at(channel)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
                addAndMakeVisible(m_outputMutes.at(channel).get());
            }
        }

        if (onBoundsRequirementChange)
            onBoundsRequirementChange();

        resized();
    }
}

void OutputControlComponent::buttonClicked(juce::Button* button)
{
    auto iter = std::find_if(m_outputMutes.begin(), m_outputMutes.end(), [=](const auto& outputMuteKV) { return outputMuteKV.second.get() == button; });
    if (iter != m_outputMutes.end() && nullptr != iter->second)
        outputMuteChange(iter->first, iter->second->getToggleState());
}

juce::Rectangle<int> OutputControlComponent::getRequiredSize()
{
    return { 60, m_channelCount * int(s_channelSize + s_channelGap) };
}


}

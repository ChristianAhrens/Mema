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

#include "AudioSelectComponent.h"

namespace Mema
{

//==============================================================================
AudioSelectComponent::AudioSelectComponent(juce::AudioDeviceManager *deviceManager,
	int minAudioInputChannels, 
	int maxAudioInputChannels, 
	int minAudioOutputChannels, 
	int maxAudioOutputChannels, 
	bool showMidiInputOptions, 
	bool showMidiOutputSelector, 
	bool showChannelsAsStereoPairs, 
	bool hideAdvancedOptionsWithButton)
	: juce::AudioDeviceSelectorComponent(*deviceManager,
		minAudioInputChannels, 
		maxAudioInputChannels, 
		minAudioOutputChannels, 
		maxAudioOutputChannels, 
		showMidiInputOptions, 
		showMidiOutputSelector, 
		showChannelsAsStereoPairs, 
		hideAdvancedOptionsWithButton)
{
	if (deviceManager)
		m_selectedAudioDeviceWhenSelectionStarted = RelevantDeviceCharacteristics(deviceManager->getCurrentAudioDevice());
}

AudioSelectComponent::~AudioSelectComponent()
{
}

void AudioSelectComponent::processAudioSelectionChanges()
{
	if (m_selectedAudioDeviceWhenSelectionStarted != RelevantDeviceCharacteristics(deviceManager.getCurrentAudioDevice()))
	{
		if (onAudioDeviceChangedDuringAudioSelection)
			onAudioDeviceChangedDuringAudioSelection();

		m_selectedAudioDeviceWhenSelectionStarted = RelevantDeviceCharacteristics(deviceManager.getCurrentAudioDevice());
	}
}

void AudioSelectComponent::paint (Graphics& g)
{
	g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker());

	g.setColour(juce::Colours::grey);
	g.drawRect(getLocalBounds(), 1);   // draw an outline around the component

	juce::AudioDeviceSelectorComponent::paint(g);
}

void AudioSelectComponent::resized()
{
	juce::AudioDeviceSelectorComponent::resized();
}

void AudioSelectComponent::visibilityChanged()
{
	if (!isVisible()) // has changed to invisible after editing
		processAudioSelectionChanges();
}

}
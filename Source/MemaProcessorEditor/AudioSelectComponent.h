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

#pragma once

#include <JuceHeader.h>

namespace Mema
{

//==============================================================================
/*
*/
class AudioSelectComponent    : public juce::AudioDeviceSelectorComponent
{
public:
	struct RelevantDeviceCharacteristics
	{
		RelevantDeviceCharacteristics()
		{
			devPtr = nullptr;
			numInputChannels = 0;
			numOutputChannels = 0;
		};
		RelevantDeviceCharacteristics(juce::AudioIODevice* d)
		{
			devPtr = d;
			numInputChannels = d->getInputChannelNames().size();
			numOutputChannels = d->getOutputChannelNames().size();
		};
		RelevantDeviceCharacteristics(juce::AudioIODevice* d, int i, int o)
		{
			devPtr = d;
			numInputChannels = i;
			numOutputChannels = o;
		};

		bool operator==(const RelevantDeviceCharacteristics& other) const
		{
			return devPtr == other.devPtr
				&& numInputChannels == other.numInputChannels
				&& numOutputChannels == other.numOutputChannels;
		};
		bool operator!=(const RelevantDeviceCharacteristics& other) const
		{
			return !(*this == other);
		};

		juce::String toString()
		{
			return juce::String::toHexString(std::uint64_t(devPtr)) + juce::String(":") + juce::String(numInputChannels) + juce::String(":") + juce::String(numOutputChannels);
		}

		juce::AudioIODevice* devPtr;
		int numInputChannels;
		int numOutputChannels;
	};

public:
    AudioSelectComponent(juce::AudioDeviceManager *deviceManager,
							int minAudioInputChannels,
							int maxAudioInputChannels,
							int minAudioOutputChannels,
							int maxAudioOutputChannels,
							bool showMidiInputOptions,
							bool showMidiOutputSelector,
							bool showChannelsAsStereoPairs,
							bool hideAdvancedOptionsWithButton);
    ~AudioSelectComponent();

	//==========================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
	void visibilityChanged() override;

	//==========================================================================
	std::function<void()> onAudioDeviceChangedDuringAudioSelection;

protected:
	//==========================================================================
	void processAudioSelectionChanges();

private:
	RelevantDeviceCharacteristics m_selectedAudioDeviceWhenSelectionStarted; // only to use to compare if user changed the device at some point!

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioSelectComponent)
};

}

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

#include "WaveformAudioComponent.h"

#include <CustomLookAndFeel.h>


namespace Mema
{

constexpr int THUMB_RES = 512;
constexpr int THUMB_TIME = 10;

//==============================================================================
WaveformAudioComponent::WaveformAudioComponent()
    : AbstractAudioVisualizer()
{
    juce::AudioFormatManager formatManager;
    formatManager.registerFormat(new juce::WavAudioFormat, true);

    m_thumbnailCache = std::make_unique<juce::AudioThumbnailCache>(THUMB_RES * 1000);
    m_thumbnail = std::make_unique<juce::AudioThumbnail>(THUMB_RES, formatManager, *m_thumbnailCache.get());

    m_buffer.clear();
    m_bufferPos = 0;
    m_bufferTime = 0;

    setUsesValuesInDB(false);
}

WaveformAudioComponent::~WaveformAudioComponent()
{
}

void WaveformAudioComponent::paint(Graphics& g)
{
    AbstractAudioVisualizer::paint(g);

    auto visuArea = getLocalBounds();
    auto legendArea = visuArea.removeFromRight(20);

    // fill visu and legend area background
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker());
    g.fillRect(visuArea);
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(legendArea);

    // get thumbnail details that internally are locking once before using multiple times later
    auto numChannels = m_thumbnail->getNumChannels();
    auto totalLength = m_thumbnail->getTotalLength();

    // draw legend, simply number the waveforms from 1..n
    if (numChannels > 0)
    {
        g.setFont(14.0f);
        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOnId));
        auto thumbWaveHeight = legendArea.getHeight() / numChannels;
        for (int i = 1; i <= numChannels; ++i)
            g.drawText(juce::String(i), legendArea.removeFromTop(thumbWaveHeight), juce::Justification::centred, true);
    }

    // draw the waveform thumbnails
    if (totalLength > 0.0)
    {
        g.setColour(getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::MeteringRmsColourId));
        m_thumbnail->drawChannels(g, visuArea, 0, THUMB_TIME, 1.0f);
    }
    else
    {
        g.setFont(14.0f);
        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOnId));
        g.drawFittedText("(No waveform data to paint)", visuArea, juce::Justification::centred, 2);
    }

    // draw moving cursor
    if (m_bufferTime > 0)
    {
        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOnId));
        auto cursorPos = int(visuArea.getWidth() * (float(m_bufferPos) / float(m_bufferTime)));
        g.drawRect(juce::Rectangle<int>(visuArea.getX() + cursorPos, visuArea.getY(), 1, visuArea.getHeight()));
    }
}

void WaveformAudioComponent::resized()
{
    AbstractAudioVisualizer::resized();
}

void WaveformAudioComponent::processingDataChanged(AbstractProcessorData* data)
{
    if (!data)
        if (!data)
            return;

    switch (data->GetDataType())
    {
    case AbstractProcessorData::AudioSignal:
    {
        ProcessorAudioSignalData* sd = static_cast<ProcessorAudioSignalData*>(data);
        if (sd->GetChannelCount() > 0)
        {
            m_bufferTime = static_cast<int>(THUMB_TIME * sd->GetSampleRate());

            if (m_thumbnail->getNumChannels() != int(sd->GetChannelCount()))
                m_thumbnail->reset(static_cast<int>(sd->GetChannelCount()), sd->GetSampleRate());
            if(m_buffer.getNumChannels() != int(sd->GetChannelCount()))
                m_buffer.setSize(static_cast<int>(sd->GetChannelCount()), m_bufferTime, false, true, true);

            for (int i = 0; i < m_buffer.getNumChannels(); ++i)
            {
                m_buffer.copyFrom(i, m_bufferPos, *sd, i, 0, sd->getNumSamples());
            }
            m_bufferPos += sd->getNumSamples();

            if (m_bufferPos >= m_bufferTime)
                m_bufferPos = 0;

            m_thumbnail->addBlock(0, m_buffer, 0, m_bufferTime);
        }
        else
            break;
        notifyChanges();
        break;
    }
    case AbstractProcessorData::Level:
    case AbstractProcessorData::Spectrum:
    case AbstractProcessorData::Invalid:
    default:
        break;
    }
}

}

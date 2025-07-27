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


//==============================================================================
WaveformAudioComponent::WaveformAudioComponent()
    : AbstractAudioVisualizer()
{
    m_waveformsComponent = std::make_unique<juce::AudioVisualiserComponent>(m_numChannels);
    m_waveformsComponent->setBufferSize(2048);
    m_waveformsComponent->setRepaintRate(20);
    addAndMakeVisible(m_waveformsComponent.get());

    lookAndFeelChanged();
}

WaveformAudioComponent::~WaveformAudioComponent()
{
}

void WaveformAudioComponent::paint(Graphics& g)
{
    AbstractAudioVisualizer::paint(g);

    auto visuArea = getLocalBounds();
    auto legendArea = visuArea.removeFromRight(m_legendWidth);

    // fill legend area background
    g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.fillRect(legendArea);

    // draw legend, simply number the waveforms from 1..n
    if (m_numChannels > 0)
    {
        g.setFont(14.0f);
        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOnId));
        auto thumbWaveHeight = legendArea.getHeight() / m_numChannels;
        for (int i = 1; i <= m_numChannels; ++i)
            g.drawText(juce::String(i), legendArea.removeFromTop(thumbWaveHeight), juce::Justification::centred, true);
    }
}

void WaveformAudioComponent::resized()
{
    auto visuArea = getLocalBounds();
    auto legendArea = visuArea.removeFromRight(m_legendWidth);

    m_waveformsComponent->setBounds(visuArea);

    AbstractAudioVisualizer::resized();
}

void WaveformAudioComponent::lookAndFeelChanged()
{
    m_waveformsComponent->setColours(
        getLookAndFeel().findColour(juce::Slider::backgroundColourId),
        getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::MeteringRmsColourId));
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
            if (m_numChannels != sd->getNumChannels())
            {
                m_numChannels = sd->getNumChannels();
                m_waveformsComponent->setNumChannels(m_numChannels);
            }
            if (m_waveformsComponent)
            {
                m_waveformsComponent->pushBuffer(*sd);
            }
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

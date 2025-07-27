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
    m_waveformsComponent = std::make_unique<juce::AudioVisualiserComponent>(m_numVisibleChannels);
    m_waveformsComponent->setBufferSize(2048);
    m_waveformsComponent->setRepaintRate(20);
    addAndMakeVisible(m_waveformsComponent.get());

    m_chNumSelButton = std::make_unique<juce::DrawableButton>("SelectChannelcount", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_chNumSelButton->setTooltip("Select number of visible channels.");
    m_chNumSelButton->onClick = [this] {
        juce::PopupMenu settingsMenu;
        for (int i = 1; i <= m_numAvailableChannels; i++)
            settingsMenu.addItem(i, juce::String(i));
        settingsMenu.showMenuAsync(juce::PopupMenu::Options(), [=](int selectedId) {
            setNumVisibleChannels(selectedId);
        });
    };
    m_chNumSelButton->setAlwaysOnTop(true);
    m_chNumSelButton->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    m_chNumSelButton->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(m_chNumSelButton.get());

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
    if (m_numVisibleChannels > 0)
    {
        g.setFont(14.0f);
        g.setColour(getLookAndFeel().findColour(juce::TextButton::textColourOnId));
        auto singleWaveformHeight = legendArea.getHeight() / m_numVisibleChannels;
        for (int i = 1; i <= m_numVisibleChannels; ++i)
            g.drawText(juce::String(i), legendArea.removeFromTop(singleWaveformHeight), juce::Justification::centred, true);
    }
}

void WaveformAudioComponent::resized()
{
    auto bounds = getLocalBounds();
    auto visuArea = bounds;
    auto legendArea = visuArea.removeFromRight(m_legendWidth);

    if (m_waveformsComponent)
        m_waveformsComponent->setBounds(visuArea);

    if (m_chNumSelButton)
        m_chNumSelButton->setBounds(bounds.removeFromBottom(22).removeFromLeft(22));

    AbstractAudioVisualizer::resized();
}

void WaveformAudioComponent::lookAndFeelChanged()
{
    if (m_waveformsComponent)
        m_waveformsComponent->setColours(
            getLookAndFeel().findColour(juce::Slider::backgroundColourId),
            getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::MeteringRmsColourId));

    auto chNumSelButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::waves24px_svg).get());
    chNumSelButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_chNumSelButton->setImages(chNumSelButtonDrawable.get());
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
            if (m_numAvailableChannels != sd->getNumChannels())
            {
                auto init = (0 == m_numAvailableChannels);
                m_numAvailableChannels = sd->getNumChannels();
                if (init)
                    setNumVisibleChannels(m_numAvailableChannels);
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

void WaveformAudioComponent::setNumVisibleChannels(int numChannels)
{
    m_numVisibleChannels = numChannels;
    if (m_waveformsComponent)
        m_waveformsComponent->setNumChannels(numChannels);
}

}

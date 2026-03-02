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

#include "SpectrumAudioComponent.h"

#include "../MemaMoAppConfiguration.h" // include to enable trigger cfg dump

#include <CustomLookAndFeel.h>


namespace Mema
{


//==============================================================================
SpectrumAudioComponent::SpectrumAudioComponent()
    : AbstractAudioVisualizer()
{
    setRefreshFrequency(20);

    m_chNumSelButton = std::make_unique<juce::DrawableButton>("SelectChannelcount", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_chNumSelButton->setTooltip("Select number of visible channels.");
    m_chNumSelButton->onClick = [this] {
        juce::PopupMenu settingsMenu;
        for (int i = 1; i <= m_numAvailableChannels; i++)
            settingsMenu.addItem(i, juce::String(i));
        settingsMenu.showMenuAsync(juce::PopupMenu::Options(), [=](int selectedId) {
            setNumVisibleChannels(selectedId);
            if (JUCEAppBasics::AppConfigurationBase::getInstance())
                JUCEAppBasics::AppConfigurationBase::getInstance()->triggerConfigurationDump(false);
        });
    };
    m_chNumSelButton->setAlwaysOnTop(true);
    m_chNumSelButton->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    m_chNumSelButton->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(m_chNumSelButton.get());

    lookAndFeelChanged();
}

SpectrumAudioComponent::~SpectrumAudioComponent()
{
}

void SpectrumAudioComponent::paint(juce::Graphics & g)
{
    AbstractAudioVisualizer::paint(g);

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // calculate what we need for our visualization area
    auto width = getWidth();
    auto height = getHeight();
    auto outerMargin = 6;
    auto channelMargin = outerMargin; // margin between channel plots
    auto channelLabelWidth = 20; // width for channel number area
    auto numVisibleChannels = getNumVisibleChannels();

    if (numVisibleChannels == 0)
        return;

    auto totalWidth = width - 2 * outerMargin - channelLabelWidth;
    auto totalHeight = height - 2 * outerMargin - outerMargin; // space for legend at bottom

    // Calculate height per channel plot including margins
    auto totalChannelMargins = (numVisibleChannels - 1) * channelMargin;
    auto availableHeightForPlots = totalHeight - totalChannelMargins;
    auto channelPlotHeight = availableHeightForPlots / numVisibleChannels;

    auto maxPlotFreq = 20000.0f;
    auto minPlotFreq = 20.0f;

    // Calculate the log scale for frequency mapping
    auto logScaleMin = log10(minPlotFreq);
    auto logScaleMax = log10(maxPlotFreq);
    auto logScaleRange = logScaleMax - logScaleMin;

    auto holdColour = getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::MeteringHoldColourId);
    auto peakColour = getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::MeteringPeakColourId);
    auto markerColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    auto legendColour = getLookAndFeel().findColour(juce::DrawableButton::textColourOnId);

    // Draw each channel in its own plot area
    for (int channelIdx = 0; channelIdx < numVisibleChannels; ++channelIdx)
    {
        if (channelIdx >= m_plotPoints.size())
            break;

        auto const& plotPoints = m_plotPoints[channelIdx];

        // Calculate this channel's plot area
        auto visuAreaY = outerMargin + channelIdx * (channelPlotHeight + channelMargin);
        juce::Rectangle<int> visuArea(outerMargin, visuAreaY, totalWidth, channelPlotHeight);

        auto visuAreaOrigX = float(outerMargin);
        auto visuAreaOrigY = float(visuAreaY + channelPlotHeight);
        auto visuAreaWidth = totalWidth;
        auto visuAreaHeight = channelPlotHeight;

        // Fill this channel's visualization area background
        g.setColour(getLookAndFeel().findColour(juce::Slider::backgroundColourId));
        g.fillRect(visuArea);

        // Draw channel number area on the right
        juce::Rectangle<int> channelLabelArea(outerMargin + totalWidth, visuAreaY, channelLabelWidth, channelPlotHeight);
        g.setColour(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
        g.fillRect(channelLabelArea);

        // Draw channel number
        g.setFont(12.0f);
        g.setColour(legendColour);
        g.drawText(juce::String(channelIdx + 1),
            channelLabelArea.toFloat(),
            juce::Justification::centred, true);

        // Draw marker lines
        auto markerLineValues = std::vector<float>{ 20.f, 30.f, 40.f, 50.f, 60.f, 70.f, 80.f, 90.f, 100.f, 200.f, 300.f, 400.f, 500.f, 600.f, 700.f, 800.f, 900.f, 1000.f, 2000.f, 3000.f, 4000.f, 5000.f, 6000.f, 7000.f, 8000.f, 9000.f, 10000.f, 20000.f };

        for (auto i = 0; i < markerLineValues.size(); ++i)
        {
            auto skewedProportionX = (log10(markerLineValues.at(i)) - logScaleMin) / logScaleRange;
            auto posX = visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);
            g.setColour(markerColour);
            g.drawLine(juce::Line<float>(posX, visuAreaOrigY, posX, visuAreaOrigY - visuAreaHeight));
        }

        // Draw dBFS label
        g.setFont(12.0f);
        g.setColour(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::menuText));
        g.drawText(juce::String(ProcessorDataAnalyzer::getGlobalMindB()) + " ... " + juce::String(ProcessorDataAnalyzer::getGlobalMaxdB()) + " dBFS",
            juce::Rectangle<float>(visuAreaOrigX + visuAreaWidth - 120.0f, float(visuAreaY), 110.0f, float(outerMargin)),
            juce::Justification::centred, true);

        // Draw spectrum curve for this channel
        if (!plotPoints.peaks.empty() && plotPoints.holds.size() == plotPoints.peaks.size())
        {
            // Helper lambda to calculate frequency for a given band index (logarithmic mapping)
            auto getBandFrequency = [&plotPoints](int bandIndex) -> float
                {
                    if (plotPoints.minFreq <= 0.0f || plotPoints.maxFreq <= plotPoints.minFreq)
                        return 0.0f;

                    float ratio = plotPoints.maxFreq / plotPoints.minFreq;
                    float t = static_cast<float>(bandIndex) / (plotPoints.peaks.size() - 1);
                    return plotPoints.minFreq * std::pow(ratio, t);
                };

            // Helper lambda to convert frequency to screen X position (logarithmic scale)
            auto frequencyToScreenX = [&](float frequency) -> float
                {
                    if (frequency <= 0.0f)
                        return visuAreaOrigX;

                    float skewedProportionX = (log10(frequency) - logScaleMin) / logScaleRange;
                    return visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);
                };

            // Find the band indices that correspond to our plot frequency range
            int minPlotIdx = -1;
            int maxPlotIdx = -1;

            for (int i = 0; i < plotPoints.peaks.size(); ++i)
            {
                float bandFreq = getBandFrequency(i);

                if (minPlotIdx == -1 && bandFreq >= minPlotFreq)
                    minPlotIdx = i;

                if (bandFreq <= maxPlotFreq)
                    maxPlotIdx = i;
            }

            // Ensure valid indices
            if (minPlotIdx == -1)
                minPlotIdx = 0;
            if (maxPlotIdx == -1 || maxPlotIdx >= plotPoints.peaks.size())
                maxPlotIdx = static_cast<int>(plotPoints.peaks.size() - 1);

            if (minPlotIdx <= maxPlotIdx)
            {
                // Build hold and peak paths
                auto holdPath = juce::Path{};
                auto peakPath = juce::Path{};

                // Start paths at first band
                float startFreq = getBandFrequency(minPlotIdx);
                float startX = frequencyToScreenX(startFreq);
                float startHoldY = visuAreaOrigY - plotPoints.holds.at(minPlotIdx) * visuAreaHeight;
                float startPeakY = visuAreaOrigY - plotPoints.peaks.at(minPlotIdx) * visuAreaHeight;

                holdPath.startNewSubPath(juce::Point<float>(startX, startHoldY));
                peakPath.startNewSubPath(juce::Point<float>(startX, startPeakY));

                // Add remaining bands
                for (int i = minPlotIdx + 1; i <= maxPlotIdx; ++i)
                {
                    float bandFreq = getBandFrequency(i);
                    float pointX = frequencyToScreenX(bandFreq);

                    float holdY = visuAreaOrigY - plotPoints.holds.at(i) * visuAreaHeight;
                    holdPath.lineTo(juce::Point<float>(pointX, holdY));

                    float peakY = visuAreaOrigY - plotPoints.peaks.at(i) * visuAreaHeight;
                    peakPath.lineTo(juce::Point<float>(pointX, peakY));
                }

                g.setColour(holdColour);
                g.strokePath(holdPath, juce::PathStrokeType(1));
                g.setColour(peakColour);
                g.strokePath(peakPath, juce::PathStrokeType(3));
            }
        }
    }

    // Draw frequency legend at the bottom
    auto markerLegendValues = std::map<float, std::string>{ {20.f, "20"}, {100.f, "100"}, {1000.f, "1k"}, {10000.f, "10k"}, {20000.f, "20k"} };
    auto legendValueWidth = 40.0f;
    auto legendY = height - outerMargin - outerMargin;

    for (auto const& [freq, label] : markerLegendValues)
    {
        auto skewedProportionX = (log10(freq) - logScaleMin) / logScaleRange;
        auto posX = float(outerMargin) + (static_cast<float>(totalWidth) * skewedProportionX);

        g.setColour(legendColour);
        g.drawText(label,
            juce::Rectangle<float>(posX - 0.5f * legendValueWidth, float(legendY), legendValueWidth, float(outerMargin)),
            juce::Justification::centred, true);
    }
}

void SpectrumAudioComponent::resized()
{
    auto bounds = getLocalBounds();
    auto visuArea = bounds;
    auto legendArea = visuArea.removeFromRight(m_legendWidth);
    ignoreUnused(legendArea);

    if (m_chNumSelButton)
        m_chNumSelButton->setBounds(bounds.removeFromTop(22).removeFromRight(22));

    AbstractAudioVisualizer::resized();
}

void SpectrumAudioComponent::lookAndFeelChanged()
{
    auto chNumSelButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::waves24px_svg).get());
    chNumSelButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_chNumSelButton->setImages(chNumSelButtonDrawable.get());
}

void SpectrumAudioComponent::processingDataChanged(AbstractProcessorData* data)
{
    if (!data)
        return;

    switch (data->GetDataType())
    {
    case AbstractProcessorData::Spectrum:
    {
        ProcessorSpectrumData* sd = static_cast<ProcessorSpectrumData*>(data);
        if (nullptr != sd && sd->GetChannelCount() > 0)
        {
            if (m_numAvailableChannels != int(sd->GetChannelCount()))
            {
                auto init = (0 == m_numAvailableChannels) && (1 == m_numVisibleChannels);
                m_numAvailableChannels = int(sd->GetChannelCount());
                if (init)
                    setNumVisibleChannels(m_numAvailableChannels);
            }

            for (auto i = 0; i < getNumVisibleChannels(); i++)
            {
                auto spectrum = sd->GetSpectrum(i);
                if (spectrum.freqRes <= 0)
                    continue;

                if (m_plotPoints[i].peaks.size() != spectrum.count)
                    m_plotPoints[i].peaks.resize(spectrum.count);
                memcpy(&m_plotPoints[i].peaks[0], &spectrum.bandsPeak[0], spectrum.count * sizeof(float));

                if (m_plotPoints[i].holds.size() != spectrum.count)
                    m_plotPoints[i].holds.resize(spectrum.count);
                memcpy(&m_plotPoints[i].holds[0], &spectrum.bandsHold[0], spectrum.count * sizeof(float));

                m_plotPoints[i].minFreq = spectrum.minFreq;
                m_plotPoints[i].maxFreq = spectrum.maxFreq;
                m_plotPoints[i].freqRes = spectrum.freqRes;
            }
        }
        else
            break;

        notifyChanges();
        break;
    }
    case AbstractProcessorData::AudioSignal:
    case AbstractProcessorData::Level:
    case AbstractProcessorData::Invalid:
    default:
        break;
    }
}

void SpectrumAudioComponent::setNumVisibleChannels(int numChannels)
{
    m_numVisibleChannels = numChannels;
    m_plotPoints.resize(numChannels);
}

int SpectrumAudioComponent::getNumVisibleChannels()
{
    return m_numVisibleChannels;
}


}

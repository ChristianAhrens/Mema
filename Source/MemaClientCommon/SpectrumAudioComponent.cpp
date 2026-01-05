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

void SpectrumAudioComponent::paint(Graphics& g)
{
    AbstractAudioVisualizer::paint(g);

    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    // calculate what we need for our center circle
    auto width = getWidth();
    auto height = getHeight();
    auto outerMargin = 6;
    auto visuAreaWidth = width - 2 * outerMargin;
    auto visuAreaHeight = height - (2 + 3) * outerMargin;
    auto maxPlotFreq = 20000.0f;
    auto minPlotFreq = 10.0f;

    juce::Rectangle<int> visuArea(outerMargin, outerMargin, visuAreaWidth, visuAreaHeight);

    // fill our visualization area background
    g.setColour(getLookAndFeel().findColour(juce::Slider::backgroundColourId));
    g.fillRect(visuArea);

    auto visuAreaOrigX = float(outerMargin);
    auto visuAreaOrigY = float(outerMargin + visuAreaHeight);

    // draw marker lines 10Hz, 100Hz, 1000Hz, 10000Hz
    auto markerColour = getLookAndFeel().findColour(juce::DrawableButton::backgroundColourId);
    auto legendColour = getLookAndFeel().findColour(juce::DrawableButton::textColourOnId);
    auto markerLineValues = std::vector<float>{ 10.f, 20.f, 30.f, 40.f, 50.f, 60.f, 70.f, 80.f, 90.f, 100.f, 200.f, 300.f, 400.f, 500.f, 600.f, 700.f, 800.f, 900.f, 1000.f, 2000.f, 3000.f, 4000.f, 5000.f, 6000.f, 7000.f, 8000.f, 9000.f, 10000.f, 20000.f };
    auto markerLegendValues = std::map<float, std::string>{ {10.f, "10"}, {100.f, "100"}, {1000.f, "1k"}, {10000.f, "10k"}, {20000.f, "20k"} };
    auto legendValueWidth = 40.0f;
    for (auto i = 0; i < markerLineValues.size(); ++i)
    {
        auto skewedProportionX = 1.0f / (log10(markerLineValues.back()) - 1.0f) * (log10(markerLineValues.at(i)) - 1.0f);
        auto posX = visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);
        g.setColour(markerColour);
        g.drawLine(juce::Line<float>(posX, visuAreaOrigY, posX, visuAreaOrigY - visuAreaHeight));

        if (markerLegendValues.count(markerLineValues.at(i)))
        {
            g.setColour(legendColour);
            g.drawText(markerLegendValues.at(markerLineValues.at(i)), juce::Rectangle<float>(posX - 0.5f * legendValueWidth, visuAreaOrigY + 10, legendValueWidth, float(outerMargin)), juce::Justification::centred, true);
        }
    }

    // draw dBFS
    g.setFont(12.0f);
    g.setColour(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::menuText));
    g.drawText(juce::String(ProcessorDataAnalyzer::getGlobalMindB()) + " ... " + juce::String(ProcessorDataAnalyzer::getGlobalMaxdB()) + " dBFS", juce::Rectangle<float>(visuAreaOrigX + visuAreaWidth - 120.0f, float(outerMargin), 110.0f, float(outerMargin)), juce::Justification::centred, true);

    // draw rta curves
    auto holdColour = getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::MeteringHoldColourId);
    auto peakColour = getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::MeteringPeakColourId);
    for (auto const plotPoints : m_plotPoints)
    {
        if (!plotPoints.peaks.empty() && plotPoints.holds.size() == plotPoints.peaks.size())
        {
            auto minPlotIdx = jlimit(0, static_cast<int>(plotPoints.peaks.size() - 1), static_cast<int>((minPlotFreq - plotPoints.minFreq) / plotPoints.freqRes));
            auto maxPlotIdx = jlimit(0, static_cast<int>(plotPoints.peaks.size() - 1), static_cast<int>((maxPlotFreq - plotPoints.minFreq) / plotPoints.freqRes));

            // hold and peak curve
            auto holdPath = juce::Path{};
            auto peakPath = juce::Path{};
            auto skewedProportionX = 1.0f / (log10(maxPlotFreq) - 1.0f) * (log10(float(minPlotIdx + 1) * plotPoints.freqRes) - 1.0f);
            auto newPointX = visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);
            auto newHoldPointY = visuAreaOrigY - plotPoints.holds.at(minPlotIdx) * visuAreaHeight;
            holdPath.startNewSubPath(juce::Point<float>(newPointX, newHoldPointY));
            auto newPeakPointY = visuAreaOrigY - plotPoints.peaks.at(minPlotIdx) * visuAreaHeight;
            peakPath.startNewSubPath(juce::Point<float>(newPointX, newPeakPointY));
            for (int i = minPlotIdx + 1; i <= maxPlotIdx; ++i)
            {
                skewedProportionX = 1.0f / (log10(maxPlotFreq) - 1.0f) * (log10((i + 1) * plotPoints.freqRes) - 1.0f);
                newPointX = visuAreaOrigX + (static_cast<float>(visuAreaWidth) * skewedProportionX);

                newHoldPointY = visuAreaOrigY - plotPoints.holds.at(i) * visuAreaHeight;
                holdPath.lineTo(juce::Point<float>(newPointX, newHoldPointY));

                newPeakPointY = visuAreaOrigY - plotPoints.peaks.at(i) * visuAreaHeight;
                peakPath.lineTo(juce::Point<float>(newPointX, newPeakPointY));
            }
            g.setColour(holdColour);
            g.strokePath(holdPath, juce::PathStrokeType(1));
            g.setColour(peakColour);
            g.strokePath(peakPath, juce::PathStrokeType(3));
        }
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

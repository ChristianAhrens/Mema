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

#include "MemaProcessorEditor.h"

#include <JuceHeader.h>

#include "../MemaProcessorEditor/PluginControlComponent.h"
#include "../MemaProcessorEditor/InputControlComponent.h"
#include "../MemaProcessorEditor/CrosspointsControlComponent.h"
#include "../MemaProcessorEditor/OutputControlComponent.h"


namespace Mema
{

//==============================================================================
MemaProcessorEditor::MemaProcessorEditor(AudioProcessor& processor)
    : juce::AudioProcessorEditor(processor)
{
    std::function<void()> boundsRequirementChange = [=]() {
        if(m_crosspointCtrl && m_outputCtrl && m_inputCtrl)
        {
            auto requiredCrosspointsSize = m_crosspointCtrl->getRequiredSize();
            auto requiredOutputCtrlSize = m_outputCtrl->getRequiredSize();
            auto requiredInputCtrlSize = m_inputCtrl->getRequiredSize();

            auto requiredSize = requiredCrosspointsSize;

            // if the IO components require more than the central crosspoint component, take that into account here
            if (requiredCrosspointsSize.getWidth() < requiredInputCtrlSize.getWidth())
                requiredSize.setWidth(requiredInputCtrlSize.getWidth());
            if (requiredCrosspointsSize.getHeight() < requiredOutputCtrlSize.getHeight())
                requiredSize.setHeight(requiredOutputCtrlSize.getHeight());

            // expand the required size with IO component 'framing' with
            requiredSize.setWidth(requiredSize.getWidth() + requiredOutputCtrlSize.getWidth() + 1);
            requiredSize.setHeight(requiredSize.getHeight() + requiredInputCtrlSize.getHeight() + 1 + sc_pluginControlHeight + 2);
            DBG(__FUNCTION__);
            if (onEditorSizeChangeRequested)
                onEditorSizeChangeRequested(requiredSize);
        }
    };

    m_pluginControl = std::make_unique<PluginControlComponent>();
    m_pluginControl->onPluginSelected = [=](const juce::PluginDescription& pluginDescription) {
        auto memaProc = dynamic_cast<MemaProcessor*>(getAudioProcessor());
        if (memaProc)
        {
            auto success = memaProc->setPlugin(pluginDescription);
            jassert(success);
        }
    };
    m_pluginControl->onPluginEnabledChange = [=](bool enabled) {
        auto memaProc = dynamic_cast<MemaProcessor*>(getAudioProcessor());
        if (memaProc)
            memaProc->setPluginEnabledState(enabled);
    };
    m_pluginControl->onPluginPrePostChange = [=](bool post) {
        auto memaProc = dynamic_cast<MemaProcessor*>(getAudioProcessor());
        if (memaProc)
            memaProc->setPluginPrePostState(post);
        };
    m_pluginControl->onShowPluginEditor = [=]() {
        auto memaProc = dynamic_cast<MemaProcessor*>(getAudioProcessor());
        if (memaProc)
            memaProc->openPluginEditor();
    };
    m_pluginControl->onClearPlugin = [=]() {
        auto memaProc = dynamic_cast<MemaProcessor*>(getAudioProcessor());
        if (memaProc)
            memaProc->clearPlugin();
    };
    addAndMakeVisible(m_pluginControl.get());

    m_ioLabel = std::make_unique<IOLabelComponent>(IOLabelComponent::Direction::OI);
    addAndMakeVisible(m_ioLabel.get());

    m_resetToUnityButton = std::make_unique<juce::DrawableButton>("UnityReset", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
    m_resetToUnityButton->setTooltip("Reset to unity patch and unmute all");
    m_resetToUnityButton->onClick = [this] { if (onResetToUnity) onResetToUnity(); };
    addAndMakeVisible(m_resetToUnityButton.get());

    m_inputCtrl = std::make_unique<InputControlComponent>();
    m_inputCtrl->onBoundsRequirementChange = boundsRequirementChange;
    addAndMakeVisible(m_inputCtrl.get());

    m_crosspointCtrl = std::make_unique<CrosspointsControlComponent>();
    m_crosspointCtrl->onBoundsRequirementChange = boundsRequirementChange;
    addAndMakeVisible(m_crosspointCtrl.get());

    m_outputCtrl = std::make_unique<OutputControlComponent>();
    m_outputCtrl->onBoundsRequirementChange = boundsRequirementChange;
    addAndMakeVisible(m_outputCtrl.get());

    auto memaProc = dynamic_cast<MemaProcessor*>(&processor);
    if (memaProc)
    {
        memaProc->addInputListener(m_inputCtrl.get());
        memaProc->addInputCommander(m_inputCtrl.get());

        memaProc->addCrosspointCommander(m_crosspointCtrl.get());
        
        memaProc->addOutputListener(m_outputCtrl.get());
        memaProc->addOutputCommander(m_outputCtrl.get());

        memaProc->onPluginSet = [=](const juce::PluginDescription& pluginDescription) { if (m_pluginControl) m_pluginControl->setSelectedPlugin(pluginDescription); };

        m_pluginControl->setPluginEnabled(memaProc->isPluginEnabled());
        m_pluginControl->setPluginPrePost(memaProc->isPluginPost());
        m_pluginControl->setSelectedPlugin(memaProc->getPluginDescription());
    }

    m_gridLayout.items = { juce::GridItem(*m_ioLabel), juce::GridItem(*m_inputCtrl), juce::GridItem(*m_outputCtrl), juce::GridItem(*m_crosspointCtrl) };
    m_gridLayout.rowGap.pixels = 1.0;
    m_gridLayout.columnGap.pixels = 1.0;

    setSize(800, 800);
}

MemaProcessorEditor::MemaProcessorEditor(AudioProcessor* processor)
    : MemaProcessorEditor(*processor)
{
}

MemaProcessorEditor::~MemaProcessorEditor()
{
}

void MemaProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.setColour(getLookAndFeel().findColour(juce::AlertWindow::backgroundColourId));
}

void MemaProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    m_pluginControl->setBounds(bounds.removeFromTop(sc_pluginControlHeight + 1));
    bounds.removeFromTop(1);

    auto requiredInputsHeight = m_inputCtrl->getRequiredSize().getHeight();
    auto requiredOutputsWidth = m_outputCtrl->getRequiredSize().getWidth();
    
    m_gridLayout.templateRows = { juce::Grid::TrackInfo(juce::Grid::Px(requiredInputsHeight)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_gridLayout.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Px(requiredOutputsWidth)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_gridLayout.performLayout(bounds);

    // reset to unity button shall hover centered above iolabel
    auto resetButtonBounds = m_ioLabel->getBounds();
    auto wMargin = (resetButtonBounds.getWidth() - sc_resetButtonSize) / 2;
    auto hMargin = (resetButtonBounds.getHeight() - sc_resetButtonSize) / 2;
    resetButtonBounds.reduce(wMargin, hMargin);
    m_resetToUnityButton->setBounds(resetButtonBounds);
}

void MemaProcessorEditor::lookAndFeelChanged()
{
    auto resetToUnityDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::replay_24dp_svg).get());
    resetToUnityDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_resetToUnityButton->setImages(resetToUnityDrawable.get());

    if (m_pluginControl)
        m_pluginControl->lookAndFeelChanged();

    juce::AudioProcessorEditor::lookAndFeelChanged();
}


}

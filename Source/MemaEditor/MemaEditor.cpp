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

#include "MemaEditor.h"

#include <JuceHeader.h>

#include "../MemaEditor/PluginControlComponent.h"
#include "../MemaEditor/InputControlComponent.h"
#include "../MemaEditor/CrosspointsControlComponent.h"
#include "../MemaEditor/OutputControlComponent.h"


namespace Mema
{

//==============================================================================
MemaEditor::MemaEditor(AudioProcessor& processor)
    : AudioProcessorEditor(processor)
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

            if (onSizeChangeRequested)
                onSizeChangeRequested(requiredSize);
        }
    };

    m_pluginControl = std::make_unique<PluginControlComponent>();
    addAndMakeVisible(m_pluginControl.get());

    m_ioLabel = std::make_unique<IOLabelComponent>(IOLabelComponent::Direction::OI);
    addAndMakeVisible(m_ioLabel.get());

    m_inputCtrl = std::make_unique<InputControlComponent>();
    m_inputCtrl->onBoundsRequirementChange = boundsRequirementChange;
    addAndMakeVisible(m_inputCtrl.get());

    m_crosspointCtrl = std::make_unique<CrosspointsControlComponent>();
    m_crosspointCtrl->onBoundsRequirementChange = boundsRequirementChange;
    addAndMakeVisible(m_crosspointCtrl.get());

    m_outputCtrl = std::make_unique<OutputControlComponent>();
    m_outputCtrl->onBoundsRequirementChange = boundsRequirementChange;
    addAndMakeVisible(m_outputCtrl.get());

    auto MemaProc = dynamic_cast<MemaProcessor*>(&processor);
    if (MemaProc)
    {
        MemaProc->addInputListener(m_inputCtrl.get());
        MemaProc->addInputCommander(m_inputCtrl.get());

        MemaProc->addCrosspointCommander(m_crosspointCtrl.get());
        
        MemaProc->addOutputListener(m_outputCtrl.get());
        MemaProc->addOutputCommander(m_outputCtrl.get());

        m_pluginControl->onPluginSelected = [=](const juce::PluginDescription& pluginDescription) { auto success = MemaProc->setPlugin(pluginDescription); jassert(success); };
        m_pluginControl->onPluginEnabledChange = [=](bool enabled) { MemaProc->setPluginEnabledState(enabled); };
    }

    m_gridLayout.items = { juce::GridItem(*m_ioLabel), juce::GridItem(*m_inputCtrl), juce::GridItem(*m_outputCtrl), juce::GridItem(*m_crosspointCtrl) };
    m_gridLayout.rowGap.pixels = 1.0;
    m_gridLayout.columnGap.pixels = 1.0;

    setSize(800, 800);
}

MemaEditor::MemaEditor(AudioProcessor* processor)
    : MemaEditor(*processor)
{
}

MemaEditor::~MemaEditor()
{
}

void MemaEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.setColour(getLookAndFeel().findColour(juce::AlertWindow::backgroundColourId));
}

void MemaEditor::resized()
{
    auto bounds = getLocalBounds();
    m_pluginControl->setBounds(bounds.removeFromTop(sc_pluginControlHeight + 1));
    bounds.removeFromTop(1);

    auto requiredInputsHeight = m_inputCtrl->getRequiredSize().getHeight();
    auto requiredOutputsWidth = m_outputCtrl->getRequiredSize().getWidth();
    
    m_gridLayout.templateRows = { juce::Grid::TrackInfo(juce::Grid::Px(requiredInputsHeight)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_gridLayout.templateColumns = { juce::Grid::TrackInfo(juce::Grid::Px(requiredOutputsWidth)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
    m_gridLayout.performLayout(bounds);
}

void MemaEditor::lookAndFeelChanged()
{
    if (m_pluginControl)
        m_pluginControl->lookAndFeelChanged();

    juce::AudioProcessorEditor::lookAndFeelChanged();
}

std::unique_ptr<XmlElement> MemaEditor::createStateXml()
{
    //auto activeVisuTypes = getActiveVisuTypes();
    //
    //auto activeVisualizersElement = std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::VISU));
    //for (auto i = AbstractAudioVisualizer::InvalidFirst + 1; i < AbstractAudioVisualizer::InvalidLast; ++i)
    //{
    //    AbstractAudioVisualizer::VisuType visuType = static_cast<AbstractAudioVisualizer::VisuType>(i);
    //    auto visualizerElement = static_cast<XmlElement*>(0);
    //
    //    if (m_AudioVisualizers.count(visuType) == 0)
    //    {
    //        visualizerElement = activeVisualizersElement->createNewChildElement(AbstractAudioVisualizer::VisuTypeToString(visuType));
    //    }
    //    else
    //    {
    //        visualizerElement = m_AudioVisualizers.at(visuType)->createStateXml().release();
    //        activeVisualizersElement->addChildElement(visualizerElement);
    //    }
    //
    //    if(visualizerElement)
    //    {
    //        visualizerElement->setAttribute("isActive", (activeVisuTypes.count(visuType) > 0) ? 1 : 0);
    //    }
    //}
    //
    //return std::make_unique<XmlElement>(*activeVisualizersElement);

    return std::make_unique<XmlElement>(AppConfiguration::getTagName(AppConfiguration::TagID::EDITORCONFIG));
}

bool MemaEditor::setStateXml(XmlElement* stateXml)
{
    if (!stateXml || (stateXml->getTagName() != AppConfiguration::getTagName(AppConfiguration::TagID::EDITORCONFIG)))
        return false;
    
    //std::set<AbstractAudioVisualizer::VisuType> visualizerTypes = {};
    //std::map< AbstractAudioVisualizer::VisuType, XmlElement*> visualizerXmlElements;
    //for (auto visualizerChildElement : stateXml->getChildIterator())
    //{
    //    auto visuType = AbstractAudioVisualizer::StringToVisuType(visualizerChildElement->getTagName().toStdString());
    //    auto isActiveAttributeValue = visualizerChildElement->getBoolAttribute("isActive");
    //    if (visuType < AbstractAudioVisualizer::InvalidLast && isActiveAttributeValue)
    //    {
    //        visualizerTypes.insert(visuType);
    //        visualizerXmlElements.insert(std::make_pair(visuType, visualizerChildElement));
    //    }
    //}
    //
    //onUpdateVisuTypes(visualizerTypes);
    //
    //for (auto mappingStateXml : visualizerXmlElements)
    //{
    //    if (m_AudioVisualizers.count(mappingStateXml.first) > 0)
    //        m_AudioVisualizers.at(mappingStateXml.first)->setStateXml(mappingStateXml.second);
    //}

    return true;
}

}

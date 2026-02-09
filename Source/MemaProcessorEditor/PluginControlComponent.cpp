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

#include "PluginControlComponent.h"


namespace Mema
{

//==============================================================================
PluginControlComponent::PluginControlComponent()
    : juce::Component()
{
	m_enableButton = std::make_unique<juce::DrawableButton>("Enable plug-in", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_enableButton->setTooltip("Enable plug-in");
	m_enableButton->setClickingTogglesState(true);
	m_enableButton->onClick = [this] { 
		if (onPluginEnabledChange)
			onPluginEnabledChange(m_enableButton->getToggleState());
	};
	addAndMakeVisible(m_enableButton.get());

	m_spacing1 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing1.get());

	m_postButton = std::make_unique<juce::TextButton>("Post", "Toggle plug-in pre/post");
	m_postButton->setClickingTogglesState(true);
	m_postButton->onClick = [this] {
		if (onPluginPrePostChange)
			onPluginPrePostChange(m_postButton->getToggleState());
		};
	addAndMakeVisible(m_postButton.get());

	m_spacing2 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing2.get());

	m_showEditorButton = std::make_unique<juce::TextButton>("None", "Show plug-in editor");
	m_showEditorButton->onClick = [this] {
		if (onShowPluginEditor)
			onShowPluginEditor();
	};
	addAndMakeVisible(m_showEditorButton.get());

	m_triggerSelectButton = std::make_unique<juce::DrawableButton>("Show plug-in selection menu", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_triggerSelectButton->setTooltip("Show plug-in selection menu");
	m_triggerSelectButton->onClick = [this] {
		showPluginsList(juce::Desktop::getMousePosition());
	};
	addAndMakeVisible(m_triggerSelectButton.get());

	m_spacing3 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing3.get());

	m_parameterConfigButton = std::make_unique<juce::DrawableButton>("Configure plug-in parameters", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_parameterConfigButton->setTooltip("Configure plug-in parameters");
	m_parameterConfigButton->onClick = [this] {
		showParameterConfig();
	};
	addAndMakeVisible(m_parameterConfigButton.get());

	m_spacing4 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing4.get());

	m_clearButton = std::make_unique<juce::DrawableButton>("Clear current plug-in", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_clearButton->setTooltip("Clear current plug-in");
	m_clearButton->onClick = [this] {
		if (onClearPlugin)
			onClearPlugin();
		};
	addAndMakeVisible(m_clearButton.get());

	m_pluginSelectionComponent = std::make_unique<PluginListAndSelectComponent>();
	m_pluginSelectionComponent->onPluginSelected = [=](const juce::PluginDescription& pluginDescription) {
		if (onPluginSelected)
			onPluginSelected(pluginDescription);
	};
}

PluginControlComponent::~PluginControlComponent()
{

}

void PluginControlComponent::showPluginsList(juce::Point<int> showPosition)
{
	m_pluginSelectionComponent->setVisible(true);
	m_pluginSelectionComponent->addToDesktop(juce::ComponentPeer::windowHasDropShadow);

	auto const display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
	if (nullptr != display && nullptr != m_pluginSelectionComponent)
	{
		if (display->totalArea.getHeight() < showPosition.getY() + m_pluginSelectionComponent->getHeight())
			showPosition.setY(showPosition.getY() - m_pluginSelectionComponent->getHeight() - 30);
		if (display->totalArea.getWidth() < showPosition.getX() + m_pluginSelectionComponent->getWidth())
			showPosition.setX(showPosition.getX() - m_pluginSelectionComponent->getWidth() - 30);
	}
	m_pluginSelectionComponent->setTopLeftPosition(showPosition);
}

void PluginControlComponent::setPluginEnabled(bool enabled)
{
	if (m_enableButton)
		m_enableButton->setToggleState(enabled, juce::dontSendNotification);
}

void PluginControlComponent::setPluginPrePost(bool post)
{
	if (m_postButton)
		m_postButton->setToggleState(post, juce::dontSendNotification);
}

void PluginControlComponent::setSelectedPlugin(const juce::PluginDescription& pluginDescription)
{
	m_selectedPluginDescription = pluginDescription;

	if (m_showEditorButton)
	{
		if (m_selectedPluginDescription.name.isEmpty())
			m_showEditorButton->setButtonText("None");
		else
			m_showEditorButton->setButtonText(m_selectedPluginDescription.name);
	}
}

void PluginControlComponent::setParameterInfos(const std::vector<Mema::PluginParameterInfo>& infos)
{
	if (infos.size() != m_parameterInfos.size())
		m_parameterInfos.clear();

	auto key = 0;
	for (auto const& info : infos)
	{
		if (0 < m_parameterInfos.count(key) || std::as_const(m_parameterInfos[key]) != info)
			m_parameterInfos[key] = info;
		key++;
	}
}

const std::map<int, Mema::PluginParameterInfo>& PluginControlComponent::getParameterInfos()
{
	return m_parameterInfos;
}

void PluginControlComponent::showParameterConfig()
{
	m_messageBox = std::make_unique<juce::AlertWindow>(
		"Plug-in parameter setup",
		"Select which of the available plug-in parameters should be made remote-controllable.",
		juce::MessageBoxIconType::NoIcon);

	// Create the container component
	m_messageBoxParameterTogglesContainer = std::make_unique<juce::Component>();

	// Create the toggle buttons and add them to the container
	for (auto const& parameterKV : m_parameterInfos)
	{
		m_messageBoxParameterToggleComponents[parameterKV.first] =
			std::make_unique<juce::ToggleButton>(parameterKV.second.name);
		m_messageBoxParameterToggleComponents[parameterKV.first]->setToggleState(parameterKV.second.isRemoteControllable, juce::dontSendNotification);
		m_messageBoxParameterTogglesContainer->addAndMakeVisible(m_messageBoxParameterToggleComponents[parameterKV.first].get());
	}

	// Calculate the required height based on number of parameters
	int toggleHeight = 24;
	auto totalHeight = int(m_parameterInfos.size()) * toggleHeight;
	m_messageBoxParameterTogglesContainer->setSize(300, totalHeight);

	// Create and configure the FlexBox layout
	m_messageBoxParameterTogglesFlexBox.items.clear();
	m_messageBoxParameterTogglesFlexBox.flexDirection = juce::FlexBox::Direction::column;
	m_messageBoxParameterTogglesFlexBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;

	// Add toggle buttons as FlexItems
	for (auto const& parameterKV : m_parameterInfos)
	{
		m_messageBoxParameterTogglesFlexBox.items.add(
			juce::FlexItem(*m_messageBoxParameterToggleComponents[parameterKV.first])
			.withHeight(float(toggleHeight))
			.withFlex(0));
	}

	// Perform the layout
	m_messageBoxParameterTogglesFlexBox.performLayout(m_messageBoxParameterTogglesContainer->getLocalBounds());

	// Add the container to the alert window
	m_messageBox->addCustomComponent(m_messageBoxParameterTogglesContainer.get());

	m_messageBox->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
	m_messageBox->addButton("Ok", 1, juce::KeyPress(juce::KeyPress::returnKey));

	m_messageBox->enterModalState(true, juce::ModalCallbackFunction::create([=](int returnValue) {
		if (returnValue == 1)
		{
			auto changeDetected = false;
			for (auto& parameterKV : m_parameterInfos)
			{
				auto currentState = m_messageBoxParameterToggleComponents[parameterKV.first]->getToggleState();
				auto& formerState = parameterKV.second;
				if (currentState != formerState.isRemoteControllable)
				{
					changeDetected = true;
					formerState.isRemoteControllable = currentState;
				}
			}

			if (changeDetected)
			{
				if (onPluginParametersStatusChanged)
					onPluginParametersStatusChanged();
			}
		}

		m_messageBoxParameterToggleComponents.clear();
		m_messageBoxParameterTogglesContainer.reset();
		m_messageBox.reset();
		}));
}

void PluginControlComponent::resized()
{
    auto bounds = getLocalBounds();
	auto margin = 1;

	if (m_enableButton)
		m_enableButton->setBounds(bounds.removeFromLeft(bounds.getHeight()));
	if (m_spacing1)
		m_spacing1->setBounds(bounds.removeFromLeft(margin));
	if (m_postButton)
		m_postButton->setBounds(bounds.removeFromLeft(int(1.5f * bounds.getHeight())));
	if (m_spacing2)
		m_spacing2->setBounds(bounds.removeFromLeft(margin));
	if (m_clearButton)
		m_clearButton->setBounds(bounds.removeFromRight(bounds.getHeight()));
	if (m_spacing3)
		m_spacing3->setBounds(bounds.removeFromRight(margin));
	if (m_parameterConfigButton)
		m_parameterConfigButton->setBounds(bounds.removeFromRight(bounds.getHeight()));
	if (m_spacing4)
		m_spacing4->setBounds(bounds.removeFromRight(margin));
	if (m_triggerSelectButton)
		m_triggerSelectButton->setBounds(bounds.removeFromRight(bounds.getHeight()));
	if (m_showEditorButton)
		m_showEditorButton->setBounds(bounds);
}

void PluginControlComponent::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::defaultFill));
}

void PluginControlComponent::lookAndFeelChanged()
{
	auto enableDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::power_settings_24dp_svg).get());
	enableDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
	m_enableButton->setImages(enableDrawable.get());

	auto triggerSelectDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::stat_minus_1_24dp_svg).get());
	triggerSelectDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
	m_triggerSelectButton->setImages(triggerSelectDrawable.get());

	auto parameterConfigButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::settings_24dp_svg).get());
	parameterConfigButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
	m_parameterConfigButton->setImages(parameterConfigButtonDrawable.get());

	auto clearButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::replay_24dp_svg).get());
	clearButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
	m_clearButton->setImages(clearButtonDrawable.get());

	juce::Component::lookAndFeelChanged();
}


}

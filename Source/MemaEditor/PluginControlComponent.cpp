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
	m_enableButton->onClick = [this] { if (onPluginEnabledChange) onPluginEnabledChange(m_enableButton->getState()); };
	addAndMakeVisible(m_enableButton.get());

	m_spacing1 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing1.get());

	m_showEditorButton = std::make_unique<juce::TextButton>("None", "Show plug-in editor");
	m_showEditorButton->onClick = [this] {};
	addAndMakeVisible(m_showEditorButton.get());

	m_triggerSelectButton = std::make_unique<juce::DrawableButton>("Show plug-in selection menu", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_triggerSelectButton->setTooltip("Show plug-in selection menu");
	m_triggerSelectButton->onClick = [this] {
		juce::PopupMenu menu;
		menu.addItem(1, "Dummy1");
		menu.addItem(2, "Dummy2");
		menu.addItem(3, "Dummy3");
		menu.showMenuAsync(juce::PopupMenu::Options(), [this](int idx) { DBG(juce::String(__FUNCTION__) << " " << idx); });
	};
	addAndMakeVisible(m_triggerSelectButton.get());

	m_spacing2 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing2.get());

	m_rescanButton = std::make_unique<juce::DrawableButton>("Re-scan plug-ins", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_rescanButton->setTooltip("Re-scan plug-ins");
	m_rescanButton->onClick = [this] { scanPlugins(); };
	addAndMakeVisible(m_rescanButton.get());
}

PluginControlComponent::~PluginControlComponent()
{

}

void PluginControlComponent::scanPlugins()
{
	juce::AudioPluginFormatManager formatManager;
	formatManager.addDefaultFormats();
	juce::KnownPluginList pluginList;
	juce::File deadMansPedalFile;
	juce::PropertiesFile::Options options;
	juce::PropertiesFile propertiesFile(options);
	juce::PluginListComponent pluginListComponent(formatManager, pluginList, deadMansPedalFile, &propertiesFile);
	juce::FileSearchPath directoryPaths;
	juce::PluginDirectoryScanner scanner(pluginList, *formatManager.getFormat(0), directoryPaths, true, deadMansPedalFile);

}

void PluginControlComponent::resized()
{
    auto bounds = getLocalBounds();
	auto margin = 1;

	if (m_enableButton)
		m_enableButton->setBounds(bounds.removeFromLeft(bounds.getHeight()));
	if (m_spacing1)
		m_spacing1->setBounds(bounds.removeFromLeft(margin));
	if (m_rescanButton)
		m_rescanButton->setBounds(bounds.removeFromRight(bounds.getHeight()));
	if (m_spacing2)
		m_spacing2->setBounds(bounds.removeFromRight(margin));
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

	auto rescanButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::replay_24dp_svg).get());
	rescanButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
	m_rescanButton->setImages(rescanButtonDrawable.get());

	juce::Component::lookAndFeelChanged();
}


}

/* Copyright (c) 2024-2025, Christian Ahrens
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

#include "AboutComponent.h"

AboutComponent::AboutComponent(const char* imageData, int imageDataSize)
    : juce::Component()
{
	m_appIcon = std::make_unique<juce::DrawableButton>("App Icon", juce::DrawableButton::ButtonStyle::ImageFitted);
	m_appIcon->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
	m_appIcon->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
	m_appIcon->setImages(juce::Drawable::createFromImageData(imageData, imageDataSize).get());
	addAndMakeVisible(m_appIcon.get());

	m_appInfoLabel = std::make_unique<juce::Label>("Version", juce::JUCEApplication::getInstance()->getApplicationName() + " " + juce::JUCEApplication::getInstance()->getApplicationVersion());
	m_appInfoLabel->setJustificationType(juce::Justification::centredBottom);
	m_appInfoLabel->setFont(juce::Font(juce::FontOptions(16.0, juce::Font::plain)));
	addAndMakeVisible(m_appInfoLabel.get());

	m_appRepoLink = std::make_unique<juce::HyperlinkButton>(juce::JUCEApplication::getInstance()->getApplicationName() + juce::String(" on GitHub"), URL("https://www.github.com/ChristianAhrens/Mema"));
	m_appRepoLink->setFont(juce::Font(juce::FontOptions(16.0, juce::Font::plain)), false /* do not resize */);
	m_appRepoLink->setJustificationType(juce::Justification::centredTop);
	addAndMakeVisible(m_appRepoLink.get());
}

AboutComponent::~AboutComponent()
{
}

void AboutComponent::paint(juce::Graphics &g)
{
	g.fillAll(getLookAndFeel().findColour(juce::DrawableButton::backgroundColourId));

	juce::Component::paint(g);
}

void AboutComponent::resized()
{
	auto bounds = getLocalBounds();
	auto margin = bounds.getHeight() / 8;
	bounds.reduce(margin, margin);
	auto iconBounds = bounds.removeFromTop(bounds.getHeight() / 2);
	auto infoBounds = bounds.removeFromTop(bounds.getHeight() / 2);
	auto& repoLinkBounds = bounds;

	m_appIcon->setBounds(iconBounds);
	m_appInfoLabel->setBounds(infoBounds);
	m_appRepoLink->setBounds(repoLinkBounds);
}

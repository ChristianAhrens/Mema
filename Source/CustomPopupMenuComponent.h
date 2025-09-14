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

#pragma once

#include <JuceHeader.h>


class CustomAboutItem : public juce::PopupMenu::CustomComponent
{
public:
	CustomAboutItem(juce::Component* componentToHold, juce::Rectangle<int> minIdealSize)
	{
		m_component = componentToHold;
		addAndMakeVisible(m_component);

		m_minIdealSize = minIdealSize;
	}
    ~CustomAboutItem()
	{
		if (m_component)
			m_component->setVisible(false);
	}

    void getIdealSize(int& idealWidth, int& idealHeight) override
	{
		auto resultingIdealSize = juce::Rectangle<int>(idealWidth, idealHeight);
		auto mc = juce::Desktop::getInstance().getComponent(0);
		if (mc)
		{
			auto fBounds = mc->getBounds().toFloat();
			auto h = fBounds.getHeight();
			auto w = fBounds.getWidth();
			if (h > 0.0f && w > 0.0f)
			{
				if (h > w)
				{
					w = 0.75f * w;
					h = w;
				}
				else
				{
					h = 0.75f * h;
					w = h;
				}

				resultingIdealSize = juce::Rectangle<float>(w, h).toNearestInt();
			}
		}

		if (resultingIdealSize.getWidth() < m_minIdealSize.getWidth() && resultingIdealSize.getHeight() < m_minIdealSize.getHeight())
		{
			idealWidth = m_minIdealSize.getWidth();
			idealHeight = m_minIdealSize.getHeight();
		}
		else
		{
			idealWidth = resultingIdealSize.getWidth();
			idealHeight = resultingIdealSize.getHeight();
		}
	}

    void resized() override
	{
		if (m_component)
			m_component->setBounds(getLocalBounds());
	}

private:
    juce::Component*        m_component = nullptr;
    juce::Rectangle<int>    m_minIdealSize;
};


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

#include "MemaClientConnectingComponent.h"

MemaClientConnectingComponent::MemaClientConnectingComponent()
    : juce::Component()
{
    m_startupProgressIndicator = std::make_unique<juce::ProgressBar>(m_progress, juce::ProgressBar::Style::circular);
    addAndMakeVisible(m_startupProgressIndicator.get());
}

MemaClientConnectingComponent::~MemaClientConnectingComponent()
{
}

void MemaClientConnectingComponent::resized()
{
    auto bounds = getLocalBounds();
    if (bounds.getWidth() > bounds.getHeight())
        bounds.reduce((bounds.getWidth() - bounds.getHeight()) / 2, 0);
    else
        bounds.reduce(0, (bounds.getHeight() - bounds.getWidth()) / 2);

    auto progressIndicatorBounds = bounds;
    progressIndicatorBounds.reduce(35, 35);
    m_startupProgressIndicator->setBounds(progressIndicatorBounds);

}

void MemaClientConnectingComponent::paint(Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::ColourIds::backgroundColourId));
}


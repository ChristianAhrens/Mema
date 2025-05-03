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

#include "CrosspointsControlComponent.h"


namespace Mema
{

//==============================================================================
CrosspointsControlComponent::CrosspointsControlComponent()
    : MemaCrosspointCommander()
{
    m_matrixGrid.rowGap.pixels = s_nodeGap;
    m_matrixGrid.columnGap.pixels = s_nodeGap;
}

CrosspointsControlComponent::~CrosspointsControlComponent()
{
}

void CrosspointsControlComponent::resized()
{
    m_matrixGrid.performLayout(getLocalBounds());
}

void CrosspointsControlComponent::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
}

void CrosspointsControlComponent::setCrosspointEnabledValue(int input, int output, bool enabledState)
{
    m_crosspointEnabledValues[input][output] = enabledState;
    if (1 == m_crosspointComponent.count(input) && 1 == m_crosspointComponent.at(input).count(output) && m_crosspointComponent.at(input).at(output))
        m_crosspointComponent.at(input).at(output)->setChecked(enabledState);

    repaint();
}

void CrosspointsControlComponent::setCrosspointFactorValue(int input, int output, float factor)
{
    m_crosspointFactorValues[input][output] = factor;
    if (1 == m_crosspointComponent.count(input) && 1 == m_crosspointComponent.at(input).count(output) && m_crosspointComponent.at(input).at(output))
        m_crosspointComponent.at(input).at(output)->setFactor(factor);

    repaint();
}

void CrosspointsControlComponent::setIOCount(int inputCount, int outputCount)
{
    if (m_ioCount != std::make_pair(inputCount, outputCount))
    {
        m_ioCount = std::make_pair(inputCount, outputCount);
        DBG(__FUNCTION__ << " " << inputCount << " " << outputCount);

        std::function<void(int, int)> initCrosspoint = [=](int input, int output) {
            DBG(__FUNCTION__ << " " << input << " " << output);
            m_crosspointEnabledValues[input][output] = false;
            m_crosspointFactorValues[input][output] = 1.0f;
            m_crosspointComponent[input][output] = std::make_unique<CrosspointComponent>(std::make_pair(input, output));
            m_crosspointComponent[input][output]->onCheckedChanged = [=](bool checkedState, CrosspointComponent* sender) {
                if (nullptr != sender)
                    crosspointEnabledChange(sender->getIdent().first, sender->getIdent().second, checkedState);
            };
            m_crosspointComponent[input][output]->onFactorChanged = [=](float factor, CrosspointComponent* sender) {
                if (nullptr != sender)
                    crosspointFactorChange(sender->getIdent().first, sender->getIdent().second, factor);
            };
            addAndMakeVisible(m_crosspointComponent[input][output].get());

            for (int j = m_matrixGrid.templateColumns.size(); j < input; j++)
                m_matrixGrid.templateColumns.add(juce::Grid::TrackInfo(juce::Grid::Px(s_nodeSize)));

            for (int j = m_matrixGrid.templateRows.size(); j < output; j++)
                m_matrixGrid.templateRows.add(juce::Grid::TrackInfo(juce::Grid::Px(s_nodeSize)));
            };

        m_matrixGrid.items.clear();
        m_matrixGrid.templateColumns.clear();
        m_matrixGrid.templateRows.clear();
        m_crosspointComponent.clear();

        for (int i = 1; i <= inputCount; i++)
        {
            if (1 != m_crosspointComponent.count(i))
            {
                for (int o = 1; o <= outputCount; o++)
                    initCrosspoint(i, o);
            }
            else
            {
                if (1 != m_crosspointComponent.at(i).count(outputCount))
                {
                    for (int o = 1; o <= outputCount; o++)
                    {
                        if (1 != m_crosspointComponent.at(i).count(o))
                            initCrosspoint(i, o);
                    }
                }
            }
        }

        for (int o = 1; o <= outputCount; o++)
        {
            for (int i = 1; i <= inputCount; i++)
            {
                if (m_crosspointComponent.at(i).at(o))
                    m_matrixGrid.items.add(juce::GridItem(m_crosspointComponent.at(i).at(o).get()));
                else
                    jassertfalse;
            }
        }

        if (onBoundsRequirementChange)
            onBoundsRequirementChange();
        
        resized();
    }
}

juce::Rectangle<int> CrosspointsControlComponent::getRequiredSize()
{
    if (m_crosspointComponent.size() > 0 && m_crosspointComponent.count(1) != 0 && m_crosspointComponent.at(1).size() > 0)
        return { int(m_crosspointComponent.size() * (s_nodeGap + s_nodeSize)), int(m_crosspointComponent.at(1).size() * (s_nodeGap + s_nodeSize)) };
    else
        return {};
}

}

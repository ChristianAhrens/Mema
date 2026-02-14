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

#include "PluginControlComponent.h"

#include <CustomLookAndFeel.h>
#include <ToggleStateSlider.h>
#include <MemaProcessor/ProcessorDataAnalyzer.h>


namespace Mema
{


PluginControlComponent::PluginControlComponent()
    : MemaClientControlComponentBase()
{
    m_parameterControlsGrid = std::make_unique<juce::Grid>();
    m_pluginNameLabel = std::make_unique<juce::Label>("pluginName");
    m_pluginNameLabel->setFont(m_pluginNameLabel->getFont().withHeight(25));
    m_pluginNameLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_pluginNameLabel.get());
}

PluginControlComponent::~PluginControlComponent()
{
}

void PluginControlComponent::paint(juce::Graphics& g)
{
    // solid overall background
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginControlComponent::resized()
{
    auto bounds = getLocalBounds();
    if (m_pluginNameLabel) m_pluginNameLabel->setBounds(bounds.removeFromTop(25));
    m_parameterBounds = bounds;
    rebuildLayout();
}

void PluginControlComponent::lookAndFeelChanged()
{
    for (auto const& comboKV : m_parameterValueComboBoxes)
        comboKV.second->setColour(juce::ComboBox::ColourIds::focusedOutlineColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    for (auto const& sliderKV : m_parameterValueSliders)
        sliderKV.second->setColour(juce::Slider::ColourIds::trackColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
}

void PluginControlComponent::resetCtrl()
{
    setIOCount({ 0,0 }); // irrelevant

    // what else to do to reset?
}

const std::string& PluginControlComponent::getPluginName()
{
    return m_pluginName;
}

void PluginControlComponent::setPluginName(const std::string& pluginName)
{
    if (m_pluginName != pluginName)
    {
        m_pluginName = pluginName;
        if (m_pluginNameLabel) m_pluginNameLabel->setText(m_pluginName, juce::dontSendNotification);
    }
}

const std::vector<Mema::PluginParameterInfo>& PluginControlComponent::getParameterInfos()
{
    return m_pluginParameterInfos;
}

void PluginControlComponent::setParameterInfos(const std::vector<Mema::PluginParameterInfo>& parameterInfos)
{
    if (m_pluginParameterInfos != parameterInfos)
    {
        m_pluginParameterInfos = parameterInfos;
        rebuildControls();
        rebuildLayout();
    }
}

void PluginControlComponent::setParameterValue(std::uint16_t index, std::string id, float value)
{
    if (index < m_pluginParameterInfos.size())
    {
        m_pluginParameterInfos.at(index).currentValue = value;
        m_pluginParameterInfos.at(index).id = id;

        if (m_parameterValueComboBoxes.count(index) != 0)
            m_parameterValueComboBoxes.at(index)->setSelectedId(int(value), juce::dontSendNotification);
        else if (m_parameterValueSliders.count(index) != 0)
            m_parameterValueSliders.at(index)->setValue(float(value), juce::dontSendNotification);
    }
}

void PluginControlComponent::rebuildControls()
{
    for (auto const& parameterInfo : m_pluginParameterInfos)
    {
        if (!parameterInfo.isRemoteControllable)
            continue;

        if (parameterInfo.type != ParameterControlType::Toggle)
        {
            if (m_parameterNameLabels.count(parameterInfo.index) != 0)
                m_parameterNameLabels.at(parameterInfo.index)->setText(parameterInfo.name, juce::dontSendNotification);
            else
            {
                m_parameterNameLabels[parameterInfo.index] = std::make_unique<juce::Label>(parameterInfo.id);
                m_parameterNameLabels[parameterInfo.index]->setJustificationType(juce::Justification::centredBottom);
                m_parameterNameLabels[parameterInfo.index]->setText(parameterInfo.name, juce::dontSendNotification);
                addAndMakeVisible(m_parameterNameLabels[parameterInfo.index].get());
            }
        }

        if (parameterInfo.type == ParameterControlType::Discrete)
        {
            if (m_parameterValueComboBoxes.count(parameterInfo.index) != 0)
                m_parameterValueComboBoxes.at(parameterInfo.index)->setSelectedId(int(parameterInfo.currentValue));
            else
            {
                m_parameterValueComboBoxes[parameterInfo.index] = std::make_unique<juce::ComboBox>(parameterInfo.id);
                for (int i = std::roundf(parameterInfo.minValue); i <= std::roundf(parameterInfo.maxValue); i++)
                    m_parameterValueComboBoxes[parameterInfo.index]->addItem(juce::String(i), i);
                m_parameterValueComboBoxes[parameterInfo.index]->onChange = [=]() {
                    auto pIdx = parameterInfo.index;
                    auto sId = m_parameterValueComboBoxes[pIdx]->getSelectedId();
                    m_pluginParameterInfos[pIdx].currentValue = float(sId);

                    if (onPluginParameterValueChanged)
                        onPluginParameterValueChanged(pIdx, m_pluginParameterInfos[pIdx].id.toStdString(), m_pluginParameterInfos[pIdx].currentValue);
                };
                addAndMakeVisible(m_parameterValueComboBoxes[parameterInfo.index].get());
            }
        }
        else if (parameterInfo.type == ParameterControlType::Continuous)
        {
            if (m_parameterValueSliders.count(parameterInfo.index) != 0)
                m_parameterValueSliders.at(parameterInfo.index)->setValue(int(parameterInfo.currentValue));
            else
            {
                m_parameterValueSliders[parameterInfo.index] = std::make_unique<JUCEAppBasics::ToggleStateSlider>(juce::Slider::Rotary, juce::Slider::NoTextBox);
                m_parameterValueSliders[parameterInfo.index]->setSliderStyle(juce::Slider::SliderStyle::Rotary);
                m_parameterValueSliders[parameterInfo.index]->setTogglalbe(false);
                m_parameterValueSliders[parameterInfo.index]->setRange(parameterInfo.minValue, parameterInfo.maxValue, juce::dontSendNotification);
                m_parameterValueSliders[parameterInfo.index]->setValue(parameterInfo.currentValue);
                m_parameterValueSliders[parameterInfo.index]->onValueChange = [=]() {
                    auto pIdx = parameterInfo.index;
                    auto value = float(m_parameterValueSliders[pIdx]->getValue());
                    m_pluginParameterInfos[pIdx].currentValue = value;

                    if (onPluginParameterValueChanged)
                        onPluginParameterValueChanged(pIdx, m_pluginParameterInfos[pIdx].id.toStdString(), m_pluginParameterInfos[pIdx].currentValue);
                    };
                addAndMakeVisible(m_parameterValueSliders[parameterInfo.index].get());
            }
        }
        else if (parameterInfo.type == ParameterControlType::Toggle)
        {
            if (m_parameterValueButtons.count(parameterInfo.index) != 0)
                m_parameterValueButtons.at(parameterInfo.index)->setToggleState(parameterInfo.currentValue > 0.5f, juce::dontSendNotification);
            else
            {
                m_parameterValueButtons[parameterInfo.index] = std::make_unique<juce::TextButton>(parameterInfo.name, "Toggle to enable or disable parameter.");
                m_parameterValueButtons[parameterInfo.index]->onStateChange = [=]() {
                    auto pIdx = parameterInfo.index;
                    auto value = float(m_parameterValueButtons[pIdx]->getToggleState() ? 1.0f : 0.0f);
                    m_pluginParameterInfos[pIdx].currentValue = value;

                    if (onPluginParameterValueChanged)
                        onPluginParameterValueChanged(pIdx, m_pluginParameterInfos[pIdx].id.toStdString(), m_pluginParameterInfos[pIdx].currentValue);
                    };
                addAndMakeVisible(m_parameterValueButtons[parameterInfo.index].get());
            }
        }
    }

    lookAndFeelChanged(); // trigger colour updates for all controls
}

void PluginControlComponent::rebuildLayout()
{
    auto bounds = m_parameterBounds;
    if (bounds.isEmpty())
        return;

    // Get the number of items to display
    auto itemCount = int(m_parameterValueComboBoxes.size() + m_parameterValueSliders.size() + m_parameterValueButtons.size());
    if (itemCount == 0)
        return;

    auto aspectRatio = static_cast<float>(bounds.getWidth()) / static_cast<float>(bounds.getHeight());

    int itemsPerRow = 1;

    if (itemCount == 1)
    {
        itemsPerRow = 1;
    }
    else
    {
        auto sqrtCount = std::sqrt(static_cast<float>(itemCount));

        if (aspectRatio > 1.5f) // Wide layout
        {
            itemsPerRow = static_cast<int>(std::ceil(sqrtCount * std::sqrt(aspectRatio)));
        }
        else if (aspectRatio < 0.67f) // Tall layout
        {
            // For tall layouts, prefer fewer items per row (more rows)
            itemsPerRow = static_cast<int>(std::ceil(sqrtCount * aspectRatio));
            itemsPerRow = juce::jmax(1, itemsPerRow); // Ensure at least 1
        }
        else // Roughly square layout
        {
            itemsPerRow = static_cast<int>(std::ceil(sqrtCount));
        }
    }

    itemsPerRow = juce::jmax(1, itemsPerRow);
    itemsPerRow = juce::jmin(itemCount, itemsPerRow); // Don't exceed total items

    // Calculate how many row pairs we need (each item takes 2 rows: label + control)
    int numRowPairs = static_cast<int>(std::ceil(static_cast<float>(itemCount) / static_cast<float>(itemsPerRow)));

    // Calculate maximum available space per cell
    const int labelHeight = 30;
    const int controlMargin = 5;

    int availableWidth = bounds.getWidth();
    int availableHeight = bounds.getHeight();

    // Calculate column width - divide available width by number of columns
    int columnWidth = availableWidth / itemsPerRow;

    // Calculate control row height - divide remaining height by number of row pairs, minus label height
    int controlHeight = (availableHeight / numRowPairs) - labelHeight;

    // Ensure minimum sizes
    columnWidth = juce::jmax(70, columnWidth);
    controlHeight = juce::jmax(50, controlHeight);

    // Build the grid
    m_parameterControlsGrid->templateRows.clear();
    m_parameterControlsGrid->templateColumns.clear();
    m_parameterControlsGrid->items.clear();

    // Set up rows: alternating between label rows and control rows
    for (int i = 0; i < numRowPairs; ++i)
    {
        m_parameterControlsGrid->templateRows.add(juce::Grid::TrackInfo(juce::Grid::Px(labelHeight)));
        m_parameterControlsGrid->templateRows.add(juce::Grid::TrackInfo(juce::Grid::Px(controlHeight)));
    }

    // Set up columns with calculated width
    for (int i = 0; i < itemsPerRow; ++i)
        m_parameterControlsGrid->templateColumns.add(juce::Grid::TrackInfo(juce::Grid::Px(columnWidth)));

    // Track current position in grid
    int currentItem = 0;

    // Add buttons
    for (auto const& buttonKV : m_parameterValueButtons)
    {
        auto* button = buttonKV.second.get();

        // Calculate grid position
        int row = (currentItem / itemsPerRow) * 2;
        int col = currentItem % itemsPerRow;


        // Add control (row + 1, col)
        m_parameterControlsGrid->items.add(juce::GridItem(*button)
            .withArea(row + 2, col + 1)
            .withMargin(juce::GridItem::Margin(controlMargin))
            .withHeight(30));

        currentItem++;
    }

    // Add combo boxes
    for (auto const& comboKV : m_parameterValueComboBoxes)
    {
        auto paramIndex = comboKV.first;
        auto* combo = comboKV.second.get();

        // Find corresponding label
        auto labelIter = m_parameterNameLabels.find(paramIndex);
        if (labelIter == m_parameterNameLabels.end() || !labelIter->second)
            continue;

        auto* label = labelIter->second.get();

        // Calculate grid position
        int row = (currentItem / itemsPerRow) * 2;
        int col = currentItem % itemsPerRow;

        // Add label (row, col) - Grid uses 1-based indexing
        m_parameterControlsGrid->items.add(juce::GridItem(*label)
            .withArea(row + 1, col + 1)
            .withMargin(juce::GridItem::Margin(2.0f)));

        // Add control (row + 1, col)
        m_parameterControlsGrid->items.add(juce::GridItem(*combo)
            .withArea(row + 2, col + 1)
            .withMargin(juce::GridItem::Margin(controlMargin))
            .withHeight(30));

        currentItem++;
    }

    // Add sliders
    for (auto const& sliderKV : m_parameterValueSliders)
    {
        auto paramIndex = sliderKV.first;
        auto* slider = sliderKV.second.get();

        // Find corresponding label
        auto labelIter = m_parameterNameLabels.find(paramIndex);
        if (labelIter == m_parameterNameLabels.end() || !labelIter->second)
            continue;

        auto* label = labelIter->second.get();

        // Calculate grid position
        int row = (currentItem / itemsPerRow) * 2;
        int col = currentItem % itemsPerRow;

        // Add label (row, col)
        m_parameterControlsGrid->items.add(juce::GridItem(*label)
            .withArea(row + 1, col + 1)
            .withMargin(juce::GridItem::Margin(2.0f)));

        // Add slider (row + 1, col) - centered and square
        // Use the smaller of columnWidth or controlHeight to ensure square shape
        auto sliderSize = float(juce::jmin(columnWidth, controlHeight) - (controlMargin * 2));

        m_parameterControlsGrid->items.add(juce::GridItem(*slider)
            .withArea(row + 2, col + 1)
            .withMargin(juce::GridItem::Margin(controlMargin))
            .withWidth(sliderSize)
            .withHeight(sliderSize)
            .withJustifySelf(juce::GridItem::JustifySelf::center)
            .withAlignSelf(juce::GridItem::AlignSelf::center));

        currentItem++;
    }

    // CRITICAL: Actually perform the layout with the bounds
    m_parameterControlsGrid->performLayout(bounds);
}


} // namespace Mema

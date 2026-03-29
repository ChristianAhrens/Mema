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
    auto margin = 25;
    auto bounds = getLocalBounds();
    if (m_pluginNameLabel)
        m_pluginNameLabel->setBounds(bounds.removeFromTop(margin));
    bounds.removeFromLeft(margin);
    bounds.removeFromRight(margin);
    m_parameterBounds = bounds;
    rebuildLayout();
}

void PluginControlComponent::lookAndFeelChanged()
{
    for (auto const& comboKV : m_parameterValueComboBoxes)
        comboKV.second->setColour(juce::ComboBox::ColourIds::focusedOutlineColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    for (auto const& sliderKV : m_parameterValueSliders)
        sliderKV.second->setColour(juce::Slider::ColourIds::trackColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    for (auto const& buttonKV : m_parameterValueButtons)
        buttonKV.second->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
}

void PluginControlComponent::resetCtrl()
{
    setIOCount({ 0,0 }); // irrelevant

    m_pluginName.clear();
    m_pluginParameterInfos.clear();
}

void PluginControlComponent::setControlsSize(const ControlsSize& ctrlsSize)
{
    MemaClientControlComponentBase::setControlsSize(ctrlsSize);
    
    rebuildLayout();
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
            m_parameterValueComboBoxes.at(index)->setSelectedId(int(value * (m_pluginParameterInfos.at(index).stepCount - 1) + 1), juce::dontSendNotification);
        else if (m_parameterValueSliders.count(index) != 0)
            m_parameterValueSliders.at(index)->setValue(float(value), juce::dontSendNotification);
        else if (m_parameterValueButtons.count(index) != 0)
            m_parameterValueButtons.at(index)->setToggleState(value > 0.5f, juce::dontSendNotification);
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
                m_parameterValueComboBoxes.at(parameterInfo.index)->setSelectedId(int(parameterInfo.currentValue * (parameterInfo.stepCount - 1) + 1), juce::dontSendNotification);
            else
            {
                m_parameterValueComboBoxes[parameterInfo.index] = std::make_unique<juce::ComboBox>(parameterInfo.id);
                jassert(parameterInfo.stepCount == parameterInfo.stepNames.size());
                int i = 1;
                for (auto const& stepName : parameterInfo.stepNames)
                    m_parameterValueComboBoxes[parameterInfo.index]->addItem(stepName, i++);
                m_parameterValueComboBoxes[parameterInfo.index]->setSelectedId(int(parameterInfo.currentValue * (parameterInfo.stepCount - 1) + 1), juce::dontSendNotification);
                m_parameterValueComboBoxes[parameterInfo.index]->onChange = [=]() {
                    auto pIdx = parameterInfo.index;
                    if (m_parameterValueComboBoxes.count(pIdx) > 0 && m_pluginParameterInfos.size() > pIdx)
                    {
                        auto sId = m_parameterValueComboBoxes[pIdx]->getSelectedId();
                        m_pluginParameterInfos[pIdx].currentValue = float((1.0f / (parameterInfo.stepCount - 1)) * (sId - 1));

                        if (onPluginParameterValueChanged)
                            onPluginParameterValueChanged(pIdx, m_pluginParameterInfos[pIdx].id.toStdString(), m_pluginParameterInfos[pIdx].currentValue);
                    }
                };
                addAndMakeVisible(m_parameterValueComboBoxes[parameterInfo.index].get());
            }
        }
        else if (parameterInfo.type == ParameterControlType::Continuous)
        {
            if (m_parameterValueSliders.count(parameterInfo.index) != 0)
                m_parameterValueSliders.at(parameterInfo.index)->setValue(parameterInfo.currentValue, juce::dontSendNotification);
            else
            {
                m_parameterValueSliders[parameterInfo.index] = std::make_unique<JUCEAppBasics::ToggleStateSlider>(juce::Slider::Rotary, juce::Slider::NoTextBox);
                m_parameterValueSliders[parameterInfo.index]->setSliderStyle(juce::Slider::SliderStyle::Rotary);
                m_parameterValueSliders[parameterInfo.index]->setTogglalbe(false);
                m_parameterValueSliders[parameterInfo.index]->setRange(parameterInfo.minValue, parameterInfo.maxValue);
                m_parameterValueSliders[parameterInfo.index]->setValue(parameterInfo.currentValue, juce::dontSendNotification);
                m_parameterValueSliders[parameterInfo.index]->onValueChange = [=]() {
                    auto pIdx = parameterInfo.index;
                    if (m_parameterValueSliders.count(pIdx) > 0 && m_pluginParameterInfos.size() > pIdx)
                    {
                        auto value = float(m_parameterValueSliders[pIdx]->getValue());
                        m_pluginParameterInfos[pIdx].currentValue = value;

                        if (onPluginParameterValueChanged)
                            onPluginParameterValueChanged(pIdx, m_pluginParameterInfos[pIdx].id.toStdString(), m_pluginParameterInfos[pIdx].currentValue);
                    }
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
                m_parameterValueButtons[parameterInfo.index]->setClickingTogglesState(true);
                m_parameterValueButtons[parameterInfo.index]->setToggleState(parameterInfo.currentValue > 0.5f, juce::dontSendNotification);
                m_parameterValueButtons[parameterInfo.index]->onStateChange = [=]() {
                    auto pIdx = parameterInfo.index;
                    if (m_parameterValueButtons.count(pIdx) > 0 && m_pluginParameterInfos.size() > pIdx)
                    {
                        auto value = float(m_parameterValueButtons[pIdx]->getToggleState() ? 1.0f : 0.0f);
                        m_pluginParameterInfos[pIdx].currentValue = value;

                        if (onPluginParameterValueChanged)
                            onPluginParameterValueChanged(pIdx, m_pluginParameterInfos[pIdx].id.toStdString(), m_pluginParameterInfos[pIdx].currentValue);
                    }
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

    auto itemCount = int(m_parameterValueComboBoxes.size() + m_parameterValueSliders.size() + m_parameterValueButtons.size());
    if (itemCount == 0)
        return;

    const auto gridItemControlSize = 3 * int(getControlsSize());
    const int labelHeight = 30;
    const int controlMargin = 5;
    const int rowPairHeight = labelHeight + gridItemControlSize;

    const int availableWidth = bounds.getWidth();
    const int availableHeight = bounds.getHeight();
    const float availableAspect = float(availableWidth) / float(juce::jmax(1, availableHeight));

    // Find itemsPerRow whose layout aspect ratio is closest to the available area's aspect ratio
    const int maxItemsPerRow = juce::jmin(itemCount, juce::jmax(1, availableWidth / gridItemControlSize));
    int itemsPerRow = 1;
    float bestAspectDiff = std::numeric_limits<float>::max();
    for (int n = 1; n <= maxItemsPerRow; ++n)
    {
        int rows = static_cast<int>(std::ceil(float(itemCount) / float(n)));
        float layoutAspect = float(n * gridItemControlSize) / float(juce::jmax(1, rows * rowPairHeight));
        float diff = std::abs(layoutAspect - availableAspect);
        if (diff < bestAspectDiff)
        {
            bestAspectDiff = diff;
            itemsPerRow = n;
        }
    }

    const int numRowPairs = static_cast<int>(std::ceil(float(itemCount) / float(itemsPerRow)));
    const int totalGridWidth  = itemsPerRow * gridItemControlSize;
    const int totalGridHeight = numRowPairs  * rowPairHeight;

    // Center the grid within the available bounds
    auto gridBounds = juce::Rectangle<int>(
        bounds.getX() + juce::jmax(0, (availableWidth  - totalGridWidth)  / 2),
        bounds.getY() + juce::jmax(0, (availableHeight - totalGridHeight) / 2),
        juce::jmin(totalGridWidth,  availableWidth),
        juce::jmin(totalGridHeight, availableHeight));

    // Build the grid
    m_parameterControlsGrid->templateRows.clear();
    m_parameterControlsGrid->templateColumns.clear();
    m_parameterControlsGrid->items.clear();

    for (int i = 0; i < numRowPairs; ++i)
    {
        m_parameterControlsGrid->templateRows.add(juce::Grid::TrackInfo(juce::Grid::Px(labelHeight)));
        m_parameterControlsGrid->templateRows.add(juce::Grid::TrackInfo(juce::Grid::Px(gridItemControlSize)));
    }
    for (int i = 0; i < itemsPerRow; ++i)
        m_parameterControlsGrid->templateColumns.add(juce::Grid::TrackInfo(juce::Grid::Px(gridItemControlSize)));

    int currentItem = 0;

    // Add buttons
    for (auto const& buttonKV : m_parameterValueButtons)
    {
        auto* button = buttonKV.second.get();
        int row = (currentItem / itemsPerRow) * 2;
        int col = currentItem % itemsPerRow;
        auto buttonSize = float(gridItemControlSize - (controlMargin * 2));

        m_parameterControlsGrid->items.add(juce::GridItem(*button)
            .withArea(row + 2, col + 1)
            .withMargin(juce::GridItem::Margin(controlMargin))
            .withWidth(buttonSize)
            .withHeight(buttonSize)
            .withJustifySelf(juce::GridItem::JustifySelf::center)
            .withAlignSelf(juce::GridItem::AlignSelf::center));

        currentItem++;
    }

    // Add combo boxes
    for (auto const& comboKV : m_parameterValueComboBoxes)
    {
        auto paramIndex = comboKV.first;
        auto* combo = comboKV.second.get();
        auto labelIter = m_parameterNameLabels.find(paramIndex);
        if (labelIter == m_parameterNameLabels.end() || !labelIter->second)
            continue;
        auto* label = labelIter->second.get();

        int row = (currentItem / itemsPerRow) * 2;
        int col = currentItem % itemsPerRow;

        m_parameterControlsGrid->items.add(juce::GridItem(*label)
            .withArea(row + 1, col + 1)
            .withMargin(juce::GridItem::Margin(2.0f)));
        m_parameterControlsGrid->items.add(juce::GridItem(*combo)
            .withArea(row + 2, col + 1)
            .withMargin(juce::GridItem::Margin(controlMargin))
            .withHeight(30)
            .withJustifySelf(juce::GridItem::JustifySelf::center)
            .withAlignSelf(juce::GridItem::AlignSelf::center));

        currentItem++;
    }

    // Add sliders
    for (auto const& sliderKV : m_parameterValueSliders)
    {
        auto paramIndex = sliderKV.first;
        auto* slider = sliderKV.second.get();
        auto labelIter = m_parameterNameLabels.find(paramIndex);
        if (labelIter == m_parameterNameLabels.end() || !labelIter->second)
            continue;
        auto* label = labelIter->second.get();

        int row = (currentItem / itemsPerRow) * 2;
        int col = currentItem % itemsPerRow;
        auto sliderSize = float(gridItemControlSize - (controlMargin * 2));

        m_parameterControlsGrid->items.add(juce::GridItem(*label)
            .withArea(row + 1, col + 1)
            .withMargin(juce::GridItem::Margin(2.0f)));
        m_parameterControlsGrid->items.add(juce::GridItem(*slider)
            .withArea(row + 2, col + 1)
            .withMargin(juce::GridItem::Margin(controlMargin))
            .withWidth(sliderSize)
            .withHeight(sliderSize)
            .withJustifySelf(juce::GridItem::JustifySelf::center)
            .withAlignSelf(juce::GridItem::AlignSelf::center));

        currentItem++;
    }

    m_parameterControlsGrid->performLayout(gridBounds);
}


} // namespace Mema

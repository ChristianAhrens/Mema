/* Copyright (c) 2026, Christian Ahrens
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
#include <MemaProcessor/ProcessorDataAnalyzer.h>


namespace Mema
{


// Converts a normalised Mema value to the native range expected by ParameterControlComponent.
// Toggle:    0/1 unchanged.
// Discrete:  normalised 0-1 → zero-based step index as float.
// Continuous: normalised 0-1 → [minValue, maxValue].
static float normalisedToNative(const Mema::PluginParameterInfo& pi, float normalisedValue)
{
    switch (pi.type)
    {
    case Mema::ParameterControlType::Discrete:
        return pi.stepCount > 1 ? normalisedValue * float(pi.stepCount - 1) : 0.0f;
    case Mema::ParameterControlType::Continuous:
        return pi.minValue + normalisedValue * (pi.maxValue - pi.minValue);
    default: // Toggle
        return normalisedValue;
    }
}

// Converts a native ParameterControlComponent value back to the normalised range Mema uses.
static float nativeToNormalised(const Mema::PluginParameterInfo& pi, float nativeValue)
{
    switch (pi.type)
    {
    case Mema::ParameterControlType::Discrete:
        return pi.stepCount > 1 ? nativeValue / float(pi.stepCount - 1) : 0.0f;
    case Mema::ParameterControlType::Continuous:
    {
        const float range = pi.maxValue - pi.minValue;
        return range > 0.0f ? (nativeValue - pi.minValue) / range : 0.0f;
    }
    default: // Toggle
        return nativeValue;
    }
}

// Builds a ParameterControlInfo (native range) from a PluginParameterInfo (normalised).
static JUCEAppBasics::ParameterControlInfo toParameterControlInfo(const Mema::PluginParameterInfo& src)
{
    JUCEAppBasics::ParameterControlInfo info;
    info.index      = src.index;
    info.name       = src.name;
    info.type       = static_cast<JUCEAppBasics::ParameterControlType>(static_cast<int>(src.type));
    info.minValue   = src.minValue;
    info.maxValue   = src.maxValue;
    info.stepSize   = src.stepSize;
    info.stepCount  = src.stepCount;
    info.stepNames  = src.stepNames;
    info.currentValue = normalisedToNative(src, src.currentValue);
    return info;
}


PluginControlComponent::PluginControlComponent()
    : MemaClientControlComponentBase()
{
    auto noconfigui = juce::JUCEApplication::getInstance()->getCommandLineParameters().contains("--noconfigui");

    m_enableButton = std::make_unique<juce::DrawableButton>("pluginEnable", juce::DrawableButton::ImageOnButtonBackground);
    m_enableButton->setClickingTogglesState(true);
    m_enableButton->setTooltip("Toggle plugin processing on/off");
    m_enableButton->onStateChange = [=]() {
        if (onPluginEnabledChanged)
            onPluginEnabledChanged(m_enableButton->getToggleState());
    };
    if (noconfigui)
        addChildComponent(m_enableButton.get());
    else
        addAndMakeVisible(m_enableButton.get());

    m_prePostButton = std::make_unique<juce::TextButton>("Post", "Toggle plugin pre/post matrix insertion");
    m_prePostButton->setClickingTogglesState(true);
    m_prePostButton->onStateChange = [=]() {
        if (onPluginPrePostChanged)
            onPluginPrePostChanged(m_prePostButton->getToggleState());
    };
    if (noconfigui)
        addChildComponent(m_prePostButton.get());
    else
        addAndMakeVisible(m_prePostButton.get());

    m_pluginNameLabel = std::make_unique<juce::Label>("pluginName");
    m_pluginNameLabel->setFont(m_pluginNameLabel->getFont().withHeight(25));
    m_pluginNameLabel->setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(m_pluginNameLabel.get());

    m_paramCtrl = std::make_unique<JUCEAppBasics::ParameterControlComponent>();
    m_paramCtrl->onParameterValueChanged = [this](int idx, float nativeValue) {
        if (idx < 0 || static_cast<std::size_t>(idx) >= m_pluginParameterInfos.size())
            return;
        auto& pi = m_pluginParameterInfos[static_cast<std::size_t>(idx)];
        if (!pi.isRemoteControllable)
            return;

        const float normalisedValue = nativeToNormalised(pi, nativeValue);
        pi.currentValue = normalisedValue;

        if (onPluginParameterValueChanged)
            onPluginParameterValueChanged(static_cast<std::uint16_t>(idx),
                                          pi.id.toStdString(),
                                          normalisedValue);
    };
    addAndMakeVisible(m_paramCtrl.get());
}

PluginControlComponent::~PluginControlComponent()
{
}

void PluginControlComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PluginControlComponent::resized()
{
    auto margin = 25;
    auto bounds = getLocalBounds();
    auto headerRow = bounds.removeFromTop(margin);

    const bool enableVisible  = m_enableButton  && m_enableButton->isVisible();
    const bool prePostVisible = m_prePostButton && m_prePostButton->isVisible();

    if (enableVisible || prePostVisible)
    {
        const int enableBtnWidth  = margin;
        const int prePostBtnWidth = 2 * margin;

        int totalBtnsWidth = 0;
        if (enableVisible)  totalBtnsWidth += enableBtnWidth;
        if (prePostVisible) totalBtnsWidth += prePostBtnWidth;

        const int labelWidth = juce::jmax(80, headerRow.getWidth() / 2);
        const int groupWidth = totalBtnsWidth + labelWidth;

        const int groupX = headerRow.getX() + juce::jmax(0, (headerRow.getWidth() - groupWidth) / 2);
        auto groupBounds = juce::Rectangle<int>(groupX, headerRow.getY(),
                                                juce::jmin(groupWidth, headerRow.getWidth()),
                                                headerRow.getHeight());

        if (enableVisible)
            m_enableButton->setBounds(groupBounds.removeFromLeft(enableBtnWidth).reduced(2));
        if (prePostVisible)
            m_prePostButton->setBounds(groupBounds.removeFromLeft(prePostBtnWidth).reduced(2));
        if (m_pluginNameLabel)
            m_pluginNameLabel->setBounds(groupBounds);
    }
    else
    {
        if (m_pluginNameLabel)
            m_pluginNameLabel->setBounds(headerRow);
    }

    bounds.removeFromLeft(margin);
    bounds.removeFromRight(margin);
    m_parameterBounds = bounds;

    if (m_paramCtrl)
        m_paramCtrl->setBounds(m_parameterBounds);
}

void PluginControlComponent::lookAndFeelChanged()
{
    auto textColour = getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId);

    if (m_enableButton)
    {
        auto drawable = juce::Drawable::createFromSVGString(BinaryData::power_settings_24dp_svg);
        drawable->replaceColour(juce::Colours::black, textColour);
        m_enableButton->setImages(drawable.get(), nullptr, nullptr, nullptr, drawable.get());
    }
}

void PluginControlComponent::resetCtrl()
{
    setIOCount({ 0, 0 });

    m_pluginName.clear();
    m_pluginParameterInfos.clear();

    if (m_enableButton)
        m_enableButton->setToggleState(false, juce::dontSendNotification);
    if (m_prePostButton)
        m_prePostButton->setToggleState(false, juce::dontSendNotification);
    if (m_paramCtrl)
        m_paramCtrl->setParameters({});
}

void PluginControlComponent::setControlsSize(const ControlsSize& ctrlsSize)
{
    MemaClientControlComponentBase::setControlsSize(ctrlsSize);

    if (m_paramCtrl)
        m_paramCtrl->setControlsSize(
            static_cast<JUCEAppBasics::ParameterControlComponent::ControlsSize>(static_cast<int>(ctrlsSize)));
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

        if (m_paramCtrl)
        {
            std::vector<JUCEAppBasics::ParameterControlInfo> infos;
            infos.reserve(m_pluginParameterInfos.size());
            for (const auto& pi : m_pluginParameterInfos)
                if (pi.isRemoteControllable)
                    infos.push_back(toParameterControlInfo(pi));
            m_paramCtrl->setParameters(infos);
        }
    }
}

void PluginControlComponent::setParameterValue(std::uint16_t index, std::string id, float normalisedValue)
{
    if (index >= m_pluginParameterInfos.size())
        return;

    m_pluginParameterInfos[index].currentValue = normalisedValue;
    m_pluginParameterInfos[index].id = id;

    if (m_paramCtrl)
        m_paramCtrl->setParameterValue(static_cast<int>(index),
                                       normalisedToNative(m_pluginParameterInfos[index], normalisedValue));
}

void PluginControlComponent::setPluginEnabled(bool enabled)
{
    if (m_enableButton)
        m_enableButton->setToggleState(enabled, juce::dontSendNotification);
}

void PluginControlComponent::setPluginPrePost(bool post)
{
    if (m_prePostButton)
        m_prePostButton->setToggleState(post, juce::dontSendNotification);
}


} // namespace Mema

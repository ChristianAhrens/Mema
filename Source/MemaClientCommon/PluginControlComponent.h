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

#pragma once

#include <JuceHeader.h>

#include "MemaClientControlComponentBase.h"

#include <MemaProcessor/MemaPluginParameterInfo.h>


/**
 * Fwd. Decls.
 */
namespace JUCEAppBasics
{
    class ToggleStateSlider;
}

namespace Mema
{


/** @class PluginControlComponent @brief Client-side plugin parameter control — renders sliders, combo boxes, and toggle buttons dynamically from MemaPluginParameterInfo descriptors. */
class PluginControlComponent : public MemaClientControlComponentBase
{
public:
    PluginControlComponent();
    virtual ~PluginControlComponent();

    //==============================================================================
    void paint(Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;

    //==============================================================================
    void resetCtrl() override;
    
    //==============================================================================
    void setControlsSize(const ControlsSize& ctrlsSize) override;

    //==============================================================================
    const std::string& getPluginName();
    void setPluginName(const std::string& pluginName);

    const std::vector<Mema::PluginParameterInfo>& getParameterInfos();
    void setParameterInfos(const std::vector<Mema::PluginParameterInfo>& parameterInfos);

    void setParameterValue(std::uint16_t index, std::string id, float value);

    void setPluginEnabled(bool enabled);
    void setPluginPrePost(bool post);

    //==============================================================================
    std::function<void(std::uint16_t, std::string, float)> onPluginParameterValueChanged;
    std::function<void(bool)> onPluginEnabledChanged;
    std::function<void(bool)> onPluginPrePostChanged;

protected:
    //==============================================================================
    void rebuildControls();
    void rebuildLayout();

private:
    //==============================================================================
    std::unique_ptr<juce::Grid> m_parameterControlsGrid;

    std::unique_ptr<juce::DrawableButton>                                        m_enableButton;
    std::unique_ptr<juce::TextButton>                                            m_prePostButton;
    std::unique_ptr<juce::Label>                                                m_pluginNameLabel;
    std::map<std::uint16_t, std::unique_ptr<juce::Label>>                       m_parameterNameLabels;
    std::map<std::uint16_t, std::unique_ptr<juce::TextButton>>                  m_parameterValueButtons;
    std::map<std::uint16_t, std::unique_ptr<JUCEAppBasics::ToggleStateSlider>>  m_parameterValueSliders;
    std::map<std::uint16_t, std::unique_ptr<juce::ComboBox>>                    m_parameterValueComboBoxes;

    std::string                             m_pluginName;
    std::vector<Mema::PluginParameterInfo>  m_pluginParameterInfos;

    juce::Rectangle<int>                    m_parameterBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginControlComponent)
};


}; // namespace Mema

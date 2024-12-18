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

#pragma once

#include <JuceHeader.h>


namespace Mema
{

//==============================================================================
class Spacing :public juce::Component
{
public:
    Spacing() = default;
    ~Spacing() = default;

    //==============================================================================
    void paint(Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    };
};

//==============================================================================
class PluginControlComponent :   public juce::Component
{
public:
    PluginControlComponent();
    ~PluginControlComponent();

    void scanPlugins();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;

    //==============================================================================
    std::function<void(const juce::PluginDescription&)> onPluginSelected;
    std::function<void(bool)> onPluginEnabledChange;

private:
    //==============================================================================
    std::unique_ptr<juce::DrawableButton>   m_enableButton;
    std::unique_ptr<Spacing>                m_spacing1;
    std::unique_ptr<juce::TextButton>       m_showEditorButton;
    std::unique_ptr<juce::DrawableButton>   m_triggerSelectButton;
    std::unique_ptr<Spacing>                m_spacing2;
    std::unique_ptr<juce::DrawableButton>   m_rescanButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginControlComponent)
};

}

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

#include <CustomLookAndFeel.h>

namespace Mema
{
    class Mema;
}
class LoadBar;
class NetworkHealthBar;
class EmptySpace;
class AboutComponent;


class MainComponent :   public juce::Component,
                        public juce::DarkModeSettingListener,
                        public juce::FocusChangeListener
{
public:
    enum MemaSettingsOption
    {
        LookAndFeel_First = 1,
        LookAndFeel_Automatic = LookAndFeel_First,
        LookAndFeel_Dark,
        LookAndFeel_Light,
        LookAndFeel_Last = LookAndFeel_Light,
        MeteringColour_First,
        MeteringColour_Green = MeteringColour_First,
        MeteringColour_Red,
        MeteringColour_Blue,
        MeteringColour_Pink,
        MeteringColour_Last = MeteringColour_Pink
    };

public:
    MainComponent();
    ~MainComponent() override;

    void toggleStandaloneWindow(std::optional<bool> standalone);
    bool isStandaloneWindow();

    //==============================================================================
    void darkModeSettingChanged() override;
    
    //========================================================================*
    void paint(Graphics&) override;
    void resized() override;

    //========================================================================*
    void lookAndFeelChanged() override;

    //========================================================================*
    void globalFocusChanged(Component* focusedComponent) override;

    //========================================================================*
    std::function<void()> onFocusLostWhileVisible;

private:
    //========================================================================*
    void handleSettingsMenuResult(int selectedId);
    void handleSettingsLookAndFeelMenuResult(int selectedId);
    void handleSettingsMeteringColourMenuResult(int selectedId);

    //========================================================================*
    void setMeteringColour(const juce::Colour& meteringColour);
    void applyMeteringColour();

    //========================================================================*
    std::unique_ptr<Mema::Mema>                 m_mbm;
    
    std::unique_ptr<juce::DrawableButton>       m_toggleStandaloneWindowButton;
    std::unique_ptr<juce::DrawableButton>       m_appSettingsButton;
    std::unique_ptr<juce::DrawableButton>       m_audioSettingsButton;
    std::unique_ptr<juce::DrawableButton>       m_aboutButton;
    std::unique_ptr<juce::DrawableButton>       m_powerButton;
    std::unique_ptr<EmptySpace>                 m_emptySpace;
    std::unique_ptr<LoadBar>                    m_sysLoadBar;
    std::unique_ptr<LoadBar>                    m_netHealthBar;

    std::unique_ptr<AboutComponent>             m_aboutComponent;

    std::unique_ptr<TooltipWindow>              m_toolTipWindowInstance;

    std::map<int, std::pair<std::string, int>>  m_settingsItems;

    std::unique_ptr<juce::LookAndFeel>          m_lookAndFeel;

    bool m_isStandaloneWindow = false;

    juce::Colour m_meteringColour = juce::Colours::forestgreen;
    
    static constexpr int sc_buttonSize = 26;
    static constexpr int sc_loadNetWidth = 70;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


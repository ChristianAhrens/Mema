/* Copyright (c) 2024, Christian Ahrens
 *
 * This file is part of Mema <https://github.com/ChristianAhrens/Mema>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <JuceHeader.h>

#include "MainComponent.h"

#include <CustomLookAndFeel.h>

 //==============================================================================
class MainApplication : public juce::JUCEApplication
{
public:
    //==============================================================================
    MainApplication() {}

    const String getApplicationName() override { return ProjectInfo::projectName; }
    const String getApplicationVersion() override { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override { return true; }

    //==============================================================================
    void initialise(const String& commandLine) override
    {
        mainWindow.reset(std::make_unique<MainWindow>(getApplicationName(), commandLine).release());
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        ignoreUnused(commandLine);
    }

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow : public juce::DocumentWindow, juce::DarkModeSettingListener
    {
    public:
        MainWindow(const juce::String& name, const juce::String& commandLine) : juce::DocumentWindow(name,
            juce::Desktop::getInstance().getDefaultLookAndFeel()
            .findColour(juce::ResizableWindow::backgroundColourId),
            juce::DocumentWindow::allButtons)
        {
            ignoreUnused(commandLine);

            setUsingNativeTitleBar(true);
            auto mainComponent = std::make_unique<MainComponent>();
            mainComponent->onPaletteStyleChange = [=](int paletteStyle, bool followLocalStyle) {
                m_followLocalStyle = followLocalStyle;
                applyPaletteStyle(static_cast<JUCEAppBasics::CustomLookAndFeel::PaletteStyle>(paletteStyle));
            };
            setContentOwned(mainComponent.release(), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen(true);
#elif JUCE_LINUX
            juce::Desktop::getInstance().setKioskModeComponent(getTopLevelComponent());
#else
            setResizable(true, true);
            centreWithSize(getWidth(), getHeight());
#endif

            setVisible(true);

            juce::Desktop::getInstance().addDarkModeSettingListener(this);
            darkModeSettingChanged(); // initially trigger correct colourscheme

            // use the settings menu item call infrastructure to activate dark mode per default
            if (auto mc = dynamic_cast<MainComponent*>(getContentComponent()))
                mc->applySettingsOption(MainComponent::SettingsOption::LookAndFeel_Dark);
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

        void darkModeSettingChanged() override
        {
            if (!m_followLocalStyle)
                return;

            if (juce::Desktop::getInstance().isDarkModeActive())
            {
                // go dark
                applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PS_Dark);
            }
            else
            {
                // go light
                applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PS_Light);
            }

            lookAndFeelChanged();
        }

        void applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PaletteStyle paletteStyle)
        {
            m_lookAndFeel = std::make_unique<JUCEAppBasics::CustomLookAndFeel>(paletteStyle);
            juce::Desktop::getInstance().setDefaultLookAndFeel(m_lookAndFeel.get());
        }

    private:
        std::unique_ptr<juce::LookAndFeel>  m_lookAndFeel;
        bool m_followLocalStyle = true;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(MainApplication)

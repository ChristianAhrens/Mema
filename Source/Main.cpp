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

#include <JuceHeader.h>

#include "Mema.h"
#include "MemaUIComponent.h"
#include "CustomPopupMenuComponent.h"

#if JUCE_MAC
#include <signal.h>
#endif

//==============================================================================
class MacMainMenuMenuBarModel : public juce::MenuBarModel
{
public:
    //==============================================================================
    void addMenu (int topLevelMenuIndex, const String& menuName, const juce::PopupMenu& popupMenu)
    {
        m_popupMenus[topLevelMenuIndex] = popupMenu;
        m_popupMenuNames[topLevelMenuIndex] = menuName;
    }
    
    juce::String getMenuNameForIndex(int topLevelMenuIndex)
    {
        jassert(m_popupMenuNames.count(topLevelMenuIndex) != 0);
        
        return m_popupMenuNames[topLevelMenuIndex];
    }
    
    juce::PopupMenu& getMenuRefForIndex(int topLevelMenuIndex)
    {
        jassert(m_popupMenus.count(topLevelMenuIndex) != 0);
        
        return m_popupMenus[topLevelMenuIndex];
    }
    
    //==============================================================================
    juce::StringArray getMenuBarNames() override
    {
        juce::StringArray menuBarNames;
        for (auto const& menuName : m_popupMenuNames)
            menuBarNames.add(menuName.second);
            
        return menuBarNames;
    }
    
    juce::PopupMenu getMenuForIndex (int topLevelMenuIndex, const String& menuName) override
    {
        ignoreUnused(menuName);

        if (m_popupMenus.count(topLevelMenuIndex) == 0) { jassertfalse; return {}; }
        if (m_popupMenuNames.count(topLevelMenuIndex) == 0) { jassertfalse; return {}; }
        
        return m_popupMenus[topLevelMenuIndex];
    }

    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override
    {
        if (onMenuItemSelected)
            onMenuItemSelected(menuItemID, topLevelMenuIndex);
    }

    //==============================================================================
    std::function<void(int, int)> onMenuItemSelected;
    
private:
    //==============================================================================
    std::map<int, juce::PopupMenu>  m_popupMenus;
    std::map<int, juce::String>  m_popupMenuNames;
};

//==============================================================================
class MemaApplication  : public juce::JUCEApplication
{
public:
    //==============================================================================
    MemaApplication() {};
    ~MemaApplication() {};

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return false; }

    //==============================================================================
    void initialise (const juce::String& /*commandLine*/) override
    {
#if JUCE_MAC
        // Ignore SIGPIPE globally, to prevent occasional unexpected app
        // termination when Mema.Mo instances disconnect while sending by
        // writing to socket is ongoing
        signal(SIGPIPE, SIG_IGN);
#endif
        
        m_taskbarComponent = std::make_unique<TaskbarComponent>(*this);
        m_taskbarComponent->setName("Mema taskbar icon");

        m_mema = std::make_unique<Mema::Mema>();
        m_memaUIComponent = createAndConnectMemaUIComponent();
        
#if JUCE_MAC
        m_macMainMenu = std::make_unique<MacMainMenuMenuBarModel>();
        auto optionsPopupMenu = juce::PopupMenu();
        optionsPopupMenu.addItem("Show as standalone window", true, false, [=]() {
            if (m_memaUIComponent)
            {
                m_isMainComponentVisible = false;
                m_memaUIComponent->setVisible(false);
                m_memaUIComponent->toggleStandaloneWindow(true);
                toggleVisibilty();
                updatePositionFromTrayIcon(juce::Desktop::getMousePosition());
            }
        });
        m_macMainMenu->addMenu(0, "Options", optionsPopupMenu);
        
        juce::MenuBarModel::setMacMainMenu(m_macMainMenu.get());
#elif JUCE_LINUX
        m_memaUIComponent->toggleStandaloneWindow(true);
        toggleVisibilty();
        updatePositionFromTrayIcon(juce::Desktop::getMousePosition());
#endif

    }

    void shutdown() override
    {
#if JUCE_MAC
        juce::MenuBarModel::setMacMainMenu(nullptr);
#endif
        
        m_memaUIComponent.reset();
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted (const juce::String& /*commandLine*/) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }

    std::unique_ptr < Mema::MemaUIComponent> createAndConnectMemaUIComponent()
    {
        auto memaUIComponent = std::make_unique<Mema::MemaUIComponent>();
        m_mema->onSizeChangeRequested = [=](juce::Rectangle<int> requestedSize) {
            if (m_memaUIComponent) m_memaUIComponent->handleSizeChangeRequest(requestedSize);
        };
        m_mema->onCpuUsageUpdate = [=](int loadPercent) {
            if (m_memaUIComponent) m_memaUIComponent->updateCpuUsageBar(loadPercent);
        };
        m_mema->onNetworkUsageUpdate = [=](std::map<int, std::pair<double, bool>> netLoads) {
            if (m_memaUIComponent) m_memaUIComponent->updateNetworkUsage(netLoads);
        };
        memaUIComponent->setEditorComponent(m_mema->getMemaProcessorEditor());
        memaUIComponent->setVisible(m_isMainComponentVisible);
        memaUIComponent->addToDesktop(juce::ComponentPeer::windowHasDropShadow);
        memaUIComponent->setTopLeftPosition(m_taskbarComponent->getX(), 50);
        memaUIComponent->setName(ProjectInfo::projectName);
        memaUIComponent->onFocusLostWhileVisible = [=]() {
            toggleVisibilty();
        };
        memaUIComponent->onLookAndFeelChanged = [=]() {
            if (m_mema) m_mema->propagateLookAndFeelChanged();
        };
        memaUIComponent->onSetupMenuClicked = [=]() {
            juce::PopupMenu setupMenu;
            setupMenu.addCustomItem(1, std::make_unique<CustomAboutItem>(m_mema->getDeviceSetupComponent(), juce::Rectangle<int>(300, 350)), nullptr, "Audio Device Setup");
            setupMenu.showMenuAsync(juce::PopupMenu::Options());
        };
        memaUIComponent->onDeleted = [=]() {
            m_memaUIComponent.release();
            m_memaUIComponent = nullptr;
        };

        memaUIComponent->lookAndFeelChanged();

        return std::move(memaUIComponent);
    }
        
    void toggleVisibilty()
    {
        if (m_memaUIComponent != nullptr)
        {
            m_memaUIComponent->setVisible(!m_memaUIComponent->isVisible());
            m_isMainComponentVisible = m_memaUIComponent->isVisible();

            if (m_isMainComponentVisible)
                m_memaUIComponent->grabKeyboardFocus();
        }
    }
        
    void updatePositionFromTrayIcon(juce::Point<int> showPosition)
    {
        if (m_memaUIComponent != nullptr)
        {
            auto const display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
            if (nullptr != display && nullptr != m_memaUIComponent)
            {
                if (display->totalArea.getHeight() < showPosition.getY() + m_memaUIComponent->getHeight())
                    showPosition.setY(showPosition.getY() - m_memaUIComponent->getHeight() - 30);
                if (display->totalArea.getWidth() < showPosition.getX() + m_memaUIComponent->getWidth())
                    showPosition.setX(showPosition.getX() - m_memaUIComponent->getWidth() - 30);
            }
            m_memaUIComponent->setTopLeftPosition(showPosition);
        }

        //juce::CallOutBox::launchAsynchronously(std::move(m_memaUIComponent), { showPosition, showPosition }, nullptr);
    }
        
    // Just add a simple icon to the Window system tray area or Mac menu bar..
    struct TaskbarComponent : public juce::SystemTrayIconComponent
    {
        TaskbarComponent(MemaApplication& app) : m_appRef(app)
        {
            setIconImage(juce::ImageFileFormat::loadFrom(BinaryData::grid_4x4_24dp_png, BinaryData::grid_4x4_24dp_pngSize),
                juce::ImageFileFormat::loadFrom(BinaryData::grid_4x4_24dp_png, BinaryData::grid_4x4_24dp_pngSize));
            setIconTooltip(JUCEApplication::getInstance()->getApplicationName());
        }

        void mouseDown(const juce::MouseEvent&) override
        {
            m_appRef.updatePositionFromTrayIcon(juce::Desktop::getMousePosition());

            // On OSX, there can be problems launching a menu when we're not the foreground
            // process, so just in case, we'll first make our process active, and then use a
            // timer to wait a moment before opening our menu, which gives the OS some time to
            // get its act together and bring our windows to the front.
            juce::Process::makeForegroundProcess();
            m_appRef.toggleVisibilty();
        }

    private:
        MemaApplication& m_appRef;
    };

private:

    bool m_isMainComponentVisible = false;
    std::unique_ptr<Mema::Mema>             m_mema;
    std::unique_ptr<Mema::MemaUIComponent>  m_memaUIComponent;
    std::unique_ptr<juce::Component>        m_taskbarComponent;
    
#if JUCE_MAC
    std::unique_ptr<MacMainMenuMenuBarModel>    m_macMainMenu;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaApplication)
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MemaApplication)

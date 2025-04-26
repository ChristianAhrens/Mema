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

        // a single instance of tooltip window is required and used by JUCE everywhere a tooltip is required.
        m_toolTipWindowInstance = std::make_unique<TooltipWindow>();
        
        m_taskbarComponent = std::make_unique<TaskbarComponent>([=](juce::Point<int> mousePosition) { showUiAsCalloutBox(mousePosition); });
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

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        juce::AlertWindow::showAsync(juce::MessageBoxOptions().withMessage(juce::String("Multiple instances of ") + juce::JUCEApplication::getInstance()->getApplicationName() + " (" + commandLine + ") are not supported"), nullptr);
    }

    std::unique_ptr<Mema::MemaUIComponent> createAndConnectMemaUIComponent()
    {
        DBG(__FUNCTION__);

        auto memaUIComponent = std::make_unique<Mema::MemaUIComponent>().release();
        m_mema->onEditorSizeChangeRequested = [memaUIComponent, this](juce::Rectangle<int> requestedSize) {
            DBG("onEditorSizeChangeRequested");
            m_lastRequestedEditorSize = requestedSize;
            jassert(memaUIComponent);
            if (memaUIComponent) memaUIComponent->handleEditorSizeChangeRequest(requestedSize);
        };
        m_mema->onCpuUsageUpdate = [=](int loadPercent) {
            jassert(memaUIComponent);
            if (memaUIComponent) memaUIComponent->updateCpuUsageBar(loadPercent);
        };
        m_mema->onNetworkUsageUpdate = [=](std::map<int, std::pair<double, bool>> netLoads) {
            jassert(memaUIComponent);
            if (memaUIComponent) memaUIComponent->updateNetworkUsage(netLoads);
        };
        memaUIComponent->setEditorComponent(m_mema->getMemaProcessorEditor());
        memaUIComponent->setVisible(m_isMainComponentVisible);
        memaUIComponent->addToDesktop(juce::ComponentPeer::windowHasDropShadow);
        memaUIComponent->setTopLeftPosition(m_taskbarComponent->getX(), 50);
        memaUIComponent->setName(ProjectInfo::projectName);
        memaUIComponent->onStandaloneWindowRequested = [=]() {
            DBG("onStandaloneWindowRequested");
            showUiAsStandaloneWindow();
        };
        memaUIComponent->onLookAndFeelChanged = [=]() {
            DBG("onLookAndFeelChanged");
            if (m_mema) m_mema->propagateLookAndFeelChanged();
        };
        memaUIComponent->onSetupMenuClicked = [=]() {
            DBG("onSetupMenuClicked");
            juce::PopupMenu setupMenu;
            setupMenu.addCustomItem(1, std::make_unique<CustomAboutItem>(m_mema->getDeviceSetupComponent(), juce::Rectangle<int>(300, 350)), nullptr, "Audio Device Setup");
            setupMenu.showMenuAsync(juce::PopupMenu::Options());
        };
        memaUIComponent->onDeleted = [=]() {
            DBG("onDeleted");
            if (m_mema)
            {
                m_mema->onEditorSizeChangeRequested = nullptr;
                m_mema->onCpuUsageUpdate = nullptr;
                m_mema->onNetworkUsageUpdate = nullptr;
            }

            m_memaUIComponent.release();
            m_memaUIComponent = nullptr;
        };

        memaUIComponent->handleEditorSizeChangeRequest(m_lastRequestedEditorSize);
        memaUIComponent->lookAndFeelChanged();
        memaUIComponent->grabKeyboardFocus();

        return std::unique_ptr<Mema::MemaUIComponent>(memaUIComponent);
    }

    void disconnectAndDeleteMemaUIComponent()
    {
        DBG(__FUNCTION__);

        m_mema->onEditorSizeChangeRequested = nullptr;
        m_mema->onCpuUsageUpdate = nullptr;
        m_mema->onNetworkUsageUpdate = nullptr;

        m_memaUIComponent.reset();
    }

    void showUiAsCalloutBox(juce::Point<int> mousePosition)
    {
        DBG(__FUNCTION__);

        // On OSX, there can be problems launching a menu when we're not the foreground
        // process, so just in case, we'll first make our process active,
        // and bring our windows to the front.
        juce::Process::makeForegroundProcess();

        juce::CallOutBox::launchAsynchronously(createAndConnectMemaUIComponent(), { mousePosition, mousePosition }, nullptr);
    }

    void showUiAsStandaloneWindow()
    {
        DBG(__FUNCTION__);

        m_memaUIComponent = createAndConnectMemaUIComponent();
        m_memaUIComponent->setStandaloneWindow(true);
    }
        
    // Just add a simple icon to the Window system tray area or Mac menu bar..
    struct TaskbarComponent : public juce::SystemTrayIconComponent
    {
        TaskbarComponent(std::function<void(juce::Point<int>)> callback) : onMouseDownWithPosition(callback)
        {
            setIconImage(juce::ImageFileFormat::loadFrom(BinaryData::grid_4x4_24dp_png, BinaryData::grid_4x4_24dp_pngSize),
                juce::ImageFileFormat::loadFrom(BinaryData::grid_4x4_24dp_png, BinaryData::grid_4x4_24dp_pngSize));
            setIconTooltip(JUCEApplication::getInstance()->getApplicationName());
        }

        void mouseDown(const juce::MouseEvent&) override
        {
            DBG(__FUNCTION__);

            if (onMouseDownWithPosition)
                onMouseDownWithPosition(juce::Desktop::getMousePosition());
        }

    private:
        std::function<void(juce::Point<int>)> onMouseDownWithPosition;
    };

private:
    bool m_isMainComponentVisible = false;
    juce::Rectangle<int> m_lastRequestedEditorSize;

    std::unique_ptr<Mema::Mema>             m_mema;
    std::unique_ptr<Mema::MemaUIComponent>  m_memaUIComponent;
    std::unique_ptr<juce::Component>        m_taskbarComponent;
    std::unique_ptr<TooltipWindow>          m_toolTipWindowInstance;
    
#if JUCE_MAC
    std::unique_ptr<MacMainMenuMenuBarModel>    m_macMainMenu;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaApplication)
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MemaApplication)

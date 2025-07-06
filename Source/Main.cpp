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

#include <AppConfigurationBase.h>

#if JUCE_MAC
#include <signal.h>
#endif

//==============================================================================
class MemaMacMainMenuMenuBarModel : public juce::MenuBarModel
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
    ~MemaApplication() {
        disconnectAndDeleteMemaUIComponent();

        Mema::Mema::deleteInstance();
    };

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
        
        m_taskbarComponent = std::make_unique<MemaTaskbarComponent>([=](juce::Point<int> mousePosition) { showUiAsCalloutBox(mousePosition); });
        m_taskbarComponent->setName(getApplicationName() + " taskbar icon");

        Mema::Mema::getInstance();
        
#if JUCE_MAC
        m_macMainMenu = std::make_unique<MemaMacMainMenuMenuBarModel>();
        auto optionsPopupMenu = juce::PopupMenu();
        optionsPopupMenu.addItem("Show as standalone window", true, false, [=]() {
            if (auto currentProcEditor = Mema::Mema::getInstance()->getMemaProcessorEditor())
                if (auto callout = currentProcEditor->findParentComponentOfClass<juce::CallOutBox>())
                    callout->exitModalState(0);
            juce::MessageManager::callAsync([=]() { showUiAsStandaloneWindow(); });
        });
        m_macMainMenu->addMenu(0, "Options", optionsPopupMenu);
        
        juce::MenuBarModel::setMacMainMenu(m_macMainMenu.get());
#elif JUCE_LINUX
        showUiAsStandaloneWindow();
#endif

    }

    void shutdown() override
    {
#if JUCE_MAC
        juce::MenuBarModel::setMacMainMenu(nullptr);
#endif
    }

    //==============================================================================
    std::unique_ptr<Mema::MemaUIComponent> createAndConnectMemaUIComponent()
    {
        if (!Mema::Mema::getInstanceWithoutCreating())
            return {};

        auto memaUIComponent = std::make_unique<Mema::MemaUIComponent>().release();
        Mema::Mema::getInstance()->onEditorSizeChangeRequested = [memaUIComponent, this](juce::Rectangle<int> requestedSize) {
            m_lastRequestedEditorSize = requestedSize;
            jassert(memaUIComponent);
            if (memaUIComponent) memaUIComponent->handleEditorSizeChangeRequest(requestedSize);
        };
        Mema::Mema::getInstance()->onCpuUsageUpdate = [=](int loadPercent) {
            jassert(memaUIComponent);
            if (memaUIComponent) memaUIComponent->updateCpuUsageBar(loadPercent);
        };
        Mema::Mema::getInstance()->onNetworkUsageUpdate = [=](std::map<int, std::pair<double, bool>> netLoads) {
            jassert(memaUIComponent);
            if (memaUIComponent) memaUIComponent->updateNetworkUsage(netLoads);
        };
        memaUIComponent->setEditorComponent(Mema::Mema::getInstance()->getMemaProcessorEditor());
        memaUIComponent->setVisible(m_isMainComponentVisible);
        memaUIComponent->addToDesktop(juce::ComponentPeer::windowHasDropShadow);
        memaUIComponent->setTopLeftPosition(m_taskbarComponent->getX(), 50);
        memaUIComponent->setName(ProjectInfo::projectName);
        memaUIComponent->onToggleStandaloneWindow = [=](bool standalone) {
            if (standalone)
            {
                if (auto callout = memaUIComponent->findParentComponentOfClass<juce::CallOutBox>())
                    callout->exitModalState(0);
                juce::MessageManager::callAsync([=]() { showUiAsStandaloneWindow(); });
            }
            else
            {
                if (m_memaUIComponent)
                    m_memaUIComponent->setStandaloneWindow(false);
                disconnectAndDeleteMemaUIComponent();
            }
        };
        memaUIComponent->onLookAndFeelChanged = [=]() {
            if (Mema::Mema::getInstanceWithoutCreating()) Mema::Mema::getInstance()->propagateLookAndFeelChanged();
        };
        memaUIComponent->onSetupMenuClicked = [=]() {
            if (Mema::Mema::getInstanceWithoutCreating())
            {
                juce::PopupMenu setupMenu;
                setupMenu.addCustomItem(1, std::make_unique<CustomAboutItem>(Mema::Mema::getInstance()->getDeviceSetupComponent(), juce::Rectangle<int>(300, 350)), nullptr, "Audio Device Setup");
                setupMenu.showMenuAsync(juce::PopupMenu::Options());
            }
        };
        memaUIComponent->onDeleted = [this]() {
            if (Mema::Mema::getInstanceWithoutCreating())
            {
                Mema::Mema::getInstance()->clearUICallbacks();
            }

            m_memaUIComponent.release();
            m_memaUIComponent = nullptr;
        };
        memaUIComponent->onSettingsChanged = [=]() {
            jassert(memaUIComponent);
            if (memaUIComponent && Mema::Mema::getInstanceWithoutCreating())
            {
                Mema::Mema::getInstance()->setUIConfigState(memaUIComponent->createStateXml());
                JUCEAppBasics::AppConfigurationBase::getInstance()->triggerConfigurationDump();
            }
        };

        memaUIComponent->handleEditorSizeChangeRequest(m_lastRequestedEditorSize);
        memaUIComponent->lookAndFeelChanged();
        memaUIComponent->setStateXml(Mema::Mema::getInstance()->getUIConfigState().get());
        memaUIComponent->resized();
        memaUIComponent->grabKeyboardFocus();

        return std::unique_ptr<Mema::MemaUIComponent>(memaUIComponent);
    }

    void disconnectAndDeleteMemaUIComponent()
    {
        if (Mema::Mema::getInstanceWithoutCreating())
        {
            Mema::Mema::getInstance()->onEditorSizeChangeRequested = nullptr;
            Mema::Mema::getInstance()->onCpuUsageUpdate = nullptr;
            Mema::Mema::getInstance()->onNetworkUsageUpdate = nullptr;
        }

        m_memaUIComponent.reset();
    }

    void showUiAsCalloutBox(const juce::Point<int>& positionToPointTo)
    {
        auto const display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
        jassert(display);
        auto position = display->userArea.getConstrainedPoint(positionToPointTo);
        
        // On OSX, there can be problems launching a menu when we're not the foreground
        // process, so just in case, we'll first make our process active,
        // and bring our windows to the front.
        juce::Process::makeForegroundProcess();

        juce::CallOutBox::launchAsynchronously(createAndConnectMemaUIComponent(), { position, position }, nullptr);
    }

    void showUiAsStandaloneWindow()
    {
        m_memaUIComponent.reset();
        m_memaUIComponent = createAndConnectMemaUIComponent();
        jassert(m_memaUIComponent);
        m_memaUIComponent->setStandaloneWindow(true);
        
        auto showPosition = juce::Desktop::getInstance().getMousePosition();
        auto const display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
        if (nullptr != display && nullptr != m_memaUIComponent)
        {
            if (display->userArea.getHeight() < showPosition.getY() + m_memaUIComponent->getHeight())
                showPosition.setY(showPosition.getY() - m_memaUIComponent->getHeight() - 30);
            if (display->userArea.getWidth() < showPosition.getX() + m_memaUIComponent->getWidth())
                showPosition.setX(showPosition.getX() - m_memaUIComponent->getWidth() - 30);
        }
        m_memaUIComponent->setTopLeftPosition(showPosition);
    }
        
    // Just add a simple icon to the Window system tray area or Mac menu bar..
    struct MemaTaskbarComponent : public juce::SystemTrayIconComponent
    {
        MemaTaskbarComponent(std::function<void(juce::Point<int>)> callback) : onMouseDownWithPosition(callback)
        {
            setIconImage(juce::ImageFileFormat::loadFrom(BinaryData::grid_4x4_24dp_png, BinaryData::grid_4x4_24dp_pngSize),
                juce::ImageFileFormat::loadFrom(BinaryData::grid_4x4_24dp_png, BinaryData::grid_4x4_24dp_pngSize));
            setIconTooltip(JUCEApplication::getInstance()->getApplicationName());
        }

        void mouseDown(const juce::MouseEvent&) override
        {
            if (onMouseDownWithPosition)
                onMouseDownWithPosition(juce::Desktop::getMousePosition());
        }

    private:
        std::function<void(juce::Point<int>)> onMouseDownWithPosition;
    };

private:
    bool m_isMainComponentVisible = false;
    juce::Rectangle<int> m_lastRequestedEditorSize;

    std::unique_ptr<Mema::MemaUIComponent>  m_memaUIComponent;
    std::unique_ptr<juce::Component>        m_taskbarComponent;
    std::unique_ptr<juce::TooltipWindow>    m_toolTipWindowInstance;
    
#if JUCE_MAC
    std::unique_ptr<MemaMacMainMenuMenuBarModel>    m_macMainMenu;
#endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaApplication)
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (MemaApplication)

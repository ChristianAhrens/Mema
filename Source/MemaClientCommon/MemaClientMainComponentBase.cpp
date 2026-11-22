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

#include "MemaClientMainComponentBase.h"

#include "MemaClientDiscoverComponent.h"
#include "MemaClientConnectingComponent.h"

#include <AppConfigurationBase.h>
#include <CustomLookAndFeel.h>


void MemaClientMainComponentBase::setStatus(const Status& s)
{
    m_currentStatus = s;
    juce::MessageManager::callAsync([safeThis = juce::Component::SafePointer<MemaClientMainComponentBase>(this)]() {
        if (safeThis)
            safeThis->resized();
    });
}

const MemaClientMainComponentBase::Status MemaClientMainComponentBase::getStatus()
{
    return m_currentStatus;
}

void MemaClientMainComponentBase::connectToMema()
{
    m_connectionRejected = false; // a new connection attempt supersedes a pending rejection

    if (m_connectingComponent)
        m_connectingComponent->setMasterServiceDescription(m_selectedService.description);
    if (m_discoverComponent)
        m_discoverComponent->setMasterServiceDescription(m_selectedService.description);

    setStatus(Status::Connecting);

    timerCallback(); // avoid codeclones by manually trigger the timed connection attempt once

    // restart connection attempt after 5s, in case something got stuck...
    startTimer(5000);
}

void MemaClientMainComponentBase::timerCallback()
{
    if (Status::Connecting == getStatus())
    {
        auto sl = m_discoverComponent->getAvailableServices();
        auto const& iter = std::find_if(sl.begin(), sl.end(), [=](const auto& service) { return service.description == m_selectedService.description; });
        if (iter != sl.end())
        {
            if ((m_selectedService.address != iter->address && m_selectedService.port != iter->port && m_selectedService.description != iter->description) || !m_networkConnection->isConnected())
            {
                m_selectedService = *iter;
                if (m_networkConnection)
                    m_networkConnection->ConnectToSocket(m_selectedService.address.toString(), m_selectedService.port);
            }
            else if (m_networkConnection && !m_networkConnection->isConnected())
                m_networkConnection->RetryConnectToSocket();
        }
    }
    else
        stopTimer();
}

void MemaClientMainComponentBase::handleSettingsLookAndFeelMenuResult(int selectedId)
{
    // helper internal function to avoid code clones
    std::function<void(int, int, int)> setSettingsItemsCheckState = [=](int a, int b, int c) {
        m_settingsItems[SettingsLookAndFeel_FollowHost].second = a;
        m_settingsItems[SettingsLookAndFeel_Dark].second = b;
        m_settingsItems[SettingsLookAndFeel_Light].second = c;
    };

    switch (selectedId)
    {
    case SettingsLookAndFeel_FollowHost:
        setSettingsItemsCheckState(1, 0, 0);
        if (onPaletteStyleChange && m_settingsHostLookAndFeelId != -1)
            onPaletteStyleChange(m_settingsHostLookAndFeelId, false);
        break;
    case SettingsLookAndFeel_Dark:
        setSettingsItemsCheckState(0, 1, 0);
        if (onPaletteStyleChange)
            onPaletteStyleChange(JUCEAppBasics::CustomLookAndFeel::PS_Dark, false);
        break;
    case SettingsLookAndFeel_Light:
        setSettingsItemsCheckState(0, 0, 1);
        if (onPaletteStyleChange)
            onPaletteStyleChange(JUCEAppBasics::CustomLookAndFeel::PS_Light, false);
        break;
    default:
        jassertfalse; // unknown id fed in unintentionally ?!
        break;
    }
}

void MemaClientMainComponentBase::handleSettingsFullscreenModeToggleResult()
{
    toggleFullscreenMode();
}

void MemaClientMainComponentBase::toggleFullscreenMode()
{
    auto enabled = isFullscreenEnabled();
    if (onSetFullscreenWindow)
        onSetFullscreenWindow(!enabled);
}

bool MemaClientMainComponentBase::isFullscreenEnabled()
{
#if JUCE_WINDOWS
    return juce::Desktop::getInstance().getKioskModeComponent() != nullptr;
#elif JUCE_MAC
    if (auto* topLevel = getTopLevelComponent())
        if (auto* peer = topLevel->getPeer())
            return peer->isFullScreen();

    jassertfalse;
    return false;
#else
    // iOS, Android and Linux always run fullscreen/kiosk (see MainWindow in Main.cpp)
    return true;
#endif
}

bool MemaClientMainComponentBase::keyPressed(const juce::KeyPress& key)
{
    // Ctrl+F11 on Windows/Linux, Cmd+Ctrl+F on macOS
    if (key == juce::KeyPress(juce::KeyPress::F11Key, juce::ModifierKeys::ctrlModifier, 0) ||
        key == juce::KeyPress('f', juce::ModifierKeys::commandModifier | juce::ModifierKeys::ctrlModifier, 0))
    {
        toggleFullscreenMode();
        return true;
    }
    return false;
}

void MemaClientMainComponentBase::showPairingTokenDialog()
{
    auto* aw = new juce::AlertWindow("Pairing token",
        "If the Mema instance you connect to requires a pairing token, enter it here. Leave empty if it doesn't.",
        juce::MessageBoxIconType::NoIcon);
    aw->addTextEditor("token", m_pairingToken, "Token:");
    aw->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
    aw->addButton("Set", 1, juce::KeyPress(juce::KeyPress::returnKey));

    aw->enterModalState(true, juce::ModalCallbackFunction::create([safeThis = juce::Component::SafePointer<MemaClientMainComponentBase>(this), aw](int result) {
        if (1 == result && safeThis)
        {
            safeThis->m_pairingToken = aw->getTextEditorContents("token");
            // the token is applied right here, so there is no need to feed the dumped configuration back to the watchers
            if (auto* config = JUCEAppBasics::AppConfigurationBase::getInstance())
                config->triggerConfigurationDump(false);
        }
    }), true);
}

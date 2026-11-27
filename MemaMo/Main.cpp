/* Copyright (c) 2024-2025, Christian Ahrens
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

#include <ClientApplication.h>

//==============================================================================
/** Traits of this app for the shared client application scaffold (JUCEAppBasics::ClientApplication). */
struct MemaMoAppTraits
{
    using MainComponent = ::MainComponent;
    static constexpr bool paletteChangeFollowsSystem = false;

    static void activateDarkPalette(MainComponent& mainComponent)
    {
        mainComponent.applySettingsOption(MainComponent::MemaMoSettingsOption::LookAndFeel_Dark);
    }
};

class MainApplication : public JUCEAppBasics::ClientApplication<MemaMoAppTraits>
{
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION(MainApplication)

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

#pragma once

#include <JuceHeader.h>

#include <AppConfigurationBase.h>

#define Mema_CONFIG_VERSION "1.0.0"

/**
 * @class MemaClientAppConfigurationBase
 * @brief Configuration handling shared by Mema.Mo and Mema.Re: validation of the connection and look-and-feel sections,
 *        reset to the bundled default and the reaction to an incompatible configuration version.
 *
 * The derived class provides its bundled default configuration and the validation of its own visualisation/control entries,
 * and has to call InitializeBase() in its constructor (not here, since that already needs the overridden functions).
 */
class MemaClientAppConfigurationBase : public JUCEAppBasics::AppConfigurationBase
{
public:
    // tag names of the entries both clients have in common (the app specific TagID enums return these)
    static constexpr const char* tagConnectionConfig    = "CONNECTIONCONFIG";
    static constexpr const char* tagServiceDescription  = "SERVICEDESCRIPTION";
    static constexpr const char* tagVisuConfig          = "VISUCONFIG";
    static constexpr const char* tagLookAndFeel         = "LOOKANDFEEL";

    /** @brief Returns true when the loaded XML contains all required configuration nodes. */
    bool isValid() override;

    /** @brief Resets every configuration value to its factory default and triggers a dump. */
    bool ResetToDefault();

protected:
    /** @brief Validates the base structure and the entries common to Mema.Mo and Mema.Re; the app specific part is validated by isVisuConfigValid. */
    static bool isValidClientConfiguration(const std::unique_ptr<juce::XmlElement>& xmlConfig,
        const std::function<bool(const juce::XmlElement& visuConfig)>& isVisuConfigValid);

    /** @brief Validates an XML configuration with the full set of rules of the app. */
    virtual bool isConfigurationValid(const std::unique_ptr<juce::XmlElement>& xmlConfig) = 0;

    /** @brief Returns the content of the bundled default configuration of the app. */
    virtual juce::String getDefaultConfigurationXml() = 0;

    /** @brief Called when the persisted config version differs from the current app version. */
    bool HandleConfigVersionConflict(const Version& configVersionFound) override;
};

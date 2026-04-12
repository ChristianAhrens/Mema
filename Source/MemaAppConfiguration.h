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

#include <AppConfigurationBase.h>

#define Mema_CONFIG_VERSION "1.0.0"

namespace Mema
{

/** @class MemaAppConfiguration
 *  @brief XML-backed application configuration manager for the Mema audio matrix tool.
 */
class MemaAppConfiguration : public JUCEAppBasics::AppConfigurationBase
{

public:
    /** @brief XML element tag identifiers used when serialising/deserialising the configuration. */
    enum TagID
    {
        PROCESSORCONFIG,    ///< Audio processor settings.
        DEVCONFIG,          ///< Audio device configuration.
        UICONFIG,           ///< UI layout and appearance.
        PLUGINCONFIG,       ///< Plugin host settings.
        INPUTMUTES,         ///< Per-channel input mute states.
        OUTPUTMUTES,        ///< Per-channel output mute states.
        CROSSPOINTGAINS,    ///< Crosspoint matrix gain values.
        PLUGINPARAM,        ///< Individual plugin parameter entry.
    };
    static juce::String getTagName(TagID ID)
    {
        switch(ID)
        {
        case PROCESSORCONFIG:
            return "PROCESSORCONFIG";
        case DEVCONFIG:
            return "DEVICECONFIG";
        case UICONFIG:
            return "UICONFIG";
        case PLUGINCONFIG:
            return "PLUGINCONFIG";
        case INPUTMUTES:
            return "INPUTMUTES";
        case OUTPUTMUTES:
            return "OUTPUTMUTES";
        case CROSSPOINTGAINS:
            return "CROSSPOINTGAINS";
        case PLUGINPARAM:
            return "PLUGINPARAM";
        default:
            return "INVALID";
        }
    };

    /** @brief XML attribute identifiers used alongside TagID elements. */
    enum AttributeID
    {
        ENABLED,        ///< Boolean enabled flag.
        POST,           ///< Post-matrix plugin insertion flag.
        PALETTESTYLE,   ///< Look-and-feel palette style index.
        METERINGCOLOR,  ///< Metering bar colour.
        IDX,            ///< Channel or parameter index.
        CONTROLLABLE,   ///< Whether a plugin parameter is remotely controllable.
        PARAMORDER,     ///< Comma-separated list of parameter indices defining the display order.
    };
    static juce::String getAttributeName(AttributeID ID)
    {
        switch (ID)
        {
        case ENABLED:
            return "ENABLED";
        case POST:
            return "POST";
        case PALETTESTYLE:
            return "PALETTESTYLE";
        case METERINGCOLOR:
            return "METERINGCOLOR";
        case IDX:
            return "IDX";
        case CONTROLLABLE:
            return "CONTROLLABLE";
        case PARAMORDER:
            return "PARAMORDER";
        default:
            return "-";
        }
    };

public:
    /** @brief Constructs the configuration, loading from or creating the given XML file. */
    explicit MemaAppConfiguration(const File &file);
    ~MemaAppConfiguration() override;

    /** @brief Returns true when the loaded XML contains all required configuration nodes. */
    bool isValid() override;
    /** @brief Static variant — validates an already-parsed XmlElement without a file. */
    static bool isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig);

    /** @brief Resets every value to factory defaults and triggers a dump. */
    bool ResetToDefault();

protected:
    /** @brief Called when the persisted config version differs from the current app version. */
    bool HandleConfigVersionConflict(const Version& configVersionFound) override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaAppConfiguration)
};

} // namespace Mema

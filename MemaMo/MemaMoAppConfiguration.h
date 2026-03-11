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
 * @class MemaMoAppConfiguration
 * @brief XML-backed configuration manager for the Mema.Mo monitor application.
 *
 * Persists the user's chosen Mema service (host/port), output visualization type,
 * metering colour, and look-and-feel selection across application launches.
 * Inherits the dump/watch/version-conflict infrastructure from AppConfigurationBase.
 *
 * @note Part of the **Mema tool suite**.  Mema.Mo is the monitoring companion to the
 *       Mema audio-matrix server: it receives streamed audio buffers over TCP and renders
 *       them as level meters, 2-D spatial field, waveform, or spectrum.  This configuration
 *       class stores everything Mema.Mo needs to reconnect and restore its last visual state.
 */
class MemaMoAppConfiguration : public JUCEAppBasics::AppConfigurationBase
{

public:
    /** @brief XML element tag identifiers used when serialising/deserialising the configuration. */
    enum TagID
    {
        CONNECTIONCONFIG,   ///< Root element for connection settings (host, port).
        SERVICEDESCRIPTION, ///< Stores the multicast service descriptor of the last connected Mema instance.
        VISUCONFIG,         ///< Root element for visualisation settings.
        OUTPUTVISUTYPE,     ///< Active output visualisation mode (meterbridge, 2-D field, waveform, …).
        METERINGCOLOUR,     ///< User-selected metering bar colour.
        LOOKANDFEEL,        ///< Active look-and-feel (follow host / dark / light).
    };
    static juce::String getTagName(TagID ID)
    {
        switch(ID)
        {
        case CONNECTIONCONFIG:
            return "CONNECTIONCONFIG";
        case SERVICEDESCRIPTION:
            return "SERVICEDESCRIPTION";
        case VISUCONFIG:
            return "VISUCONFIG";
        case OUTPUTVISUTYPE:
            return "OUTPUTVISUTYPE";
        case METERINGCOLOUR:
            return "METERINGCOLOUR";
        case LOOKANDFEEL:
            return "LOOKANDFEEL";
        default:
            return "INVALID";
        }
    };

    /** @brief XML attribute identifiers used alongside TagID elements. */
    enum AttributeID
    {
        ENABLED,    ///< Boolean flag indicating whether a feature or connection is active.
        COUNT,      ///< Integer storing a channel or item count.
    };
    static juce::String getAttributeName(AttributeID ID)
    {
        switch (ID)
        {
        case ENABLED:
            return "ENABLED";
        case COUNT:
            return "COUNT";
        default:
            return "-";
        }
    };

public:
    /** @brief Constructs the configuration, loading from or creating the given XML file. */
    explicit MemaMoAppConfiguration(const File &file);
    ~MemaMoAppConfiguration() override;

    /** @brief Returns true when the loaded XML contains all required Mema.Mo configuration nodes. */
    bool isValid() override;
    /** @brief Static variant — validates an already-parsed XmlElement without a file on disk. */
    static bool isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig);

    /** @brief Resets every configuration value to its factory default and triggers a dump. */
    bool ResetToDefault();

protected:
    /** @brief Called when the persisted config version differs from the current app version. */
    bool HandleConfigVersionConflict(const Version& configVersionFound) override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaMoAppConfiguration)
};


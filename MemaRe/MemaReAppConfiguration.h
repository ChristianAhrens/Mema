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
 * @class MemaReAppConfiguration
 * @brief XML-backed configuration manager for the Mema.Re remote-control application.
 *
 * Persists the user's chosen Mema service, active control format, accent colour,
 * look-and-feel, control element size, and ADM-OSC external-control settings across
 * application launches.
 *
 * @note Part of the **Mema tool suite**.  Mema.Re is the remote-control companion to
 *       the Mema audio-matrix server.  Unlike Mema.Mo (which only receives audio data),
 *       Mema.Re sends `ControlParametersMessage` updates back to Mema over the same
 *       TCP connection and can also receive ADM-OSC UDP packets from an external
 *       spatial-audio controller (e.g. Grapes).
 */
class MemaReAppConfiguration : public JUCEAppBasics::AppConfigurationBase
{

public:
    /** @brief XML element tag identifiers used when serialising/deserialising the configuration. */
    enum TagID
    {
        CONNECTIONCONFIG,   ///< Root element for TCP connection settings.
        SERVICEDESCRIPTION, ///< Stored multicast service descriptor of the last connected Mema instance.
        VISUCONFIG,         ///< Root element for visualisation/control settings.
        CONTROLFORMAT,      ///< Active control mode (faderbank, 2-D panning layout, plugin parameters).
        CONTROLCOLOUR,      ///< User-selected accent colour for control elements.
        LOOKANDFEEL,        ///< Active look-and-feel (follow host / dark / light).
        CONTROLSSIZE,       ///< Size category for control widgets (S / M / L).
        EXTCTRLCONFIG,      ///< Root element for external ADM-OSC controller settings.
        ADMOSCHOST,         ///< ADM-OSC host (local UDP listen port).
        ADMOSCCLIENT,       ///< ADM-OSC client (remote IP and port for outgoing messages).
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
        case CONTROLFORMAT:
            return "CONTROLFORMAT";
        case CONTROLCOLOUR:
            return "CONTROLCOLOUR";
        case LOOKANDFEEL:
            return "LOOKANDFEEL";
        case CONTROLSSIZE:
            return "CONTROLSSIZE";
        case EXTCTRLCONFIG:
            return "EXTCTRLCONFIG";
        case ADMOSCHOST:
            return "ADMOSCHOST";
        case ADMOSCCLIENT:
            return "ADMOSCCLIENT";
        default:
            return "INVALID";
        }
    };

    /** @brief XML attribute identifiers used alongside TagID elements. */
    enum AttributeID
    {
        ENABLED,    ///< Boolean flag indicating whether a feature or connection is active.
        IP,         ///< IP address string (used for ADM-OSC remote client address).
        PORT,       ///< UDP/TCP port number.
    };
    static juce::String getAttributeName(AttributeID ID)
    {
        switch (ID)
        {
        case ENABLED:
            return "ENABLED";
        case IP:
            return "IP";
        case PORT:
            return "PORT";
        default:
            return "-";
        }
    };

public:
    /** @brief Constructs the configuration, loading from or creating the given XML file. */
    explicit MemaReAppConfiguration(const File &file);
    ~MemaReAppConfiguration() override;

    /** @brief Returns true when the loaded XML contains all required Mema.Re configuration nodes. */
    bool isValid() override;
    /** @brief Static variant — validates an already-parsed XmlElement without a file on disk. */
    static bool isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig);

    /** @brief Resets every configuration value to its factory default and triggers a dump. */
    bool ResetToDefault();

protected:
    /** @brief Called when the persisted config version differs from the current app version. */
    bool HandleConfigVersionConflict(const Version& configVersionFound) override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaReAppConfiguration)
};


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


class MemaReAppConfiguration : public JUCEAppBasics::AppConfigurationBase
{

public:
    enum TagID
    {
        CONNECTIONCONFIG,
        SERVICEDESCRIPTION,
        VISUCONFIG,
        OUTPUTVISUTYPE,
        METERINGCOLOUR,
        LOOKANDFEEL,
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

    enum AttributeID
    {
        ENABLED,
    };
    static juce::String getAttributeName(AttributeID ID)
    {
        switch (ID)
        {
        case ENABLED:
            return "ENABLED";
        default:
            return "-";
        }
    };

public:
    explicit MemaReAppConfiguration(const File &file);
    ~MemaReAppConfiguration() override;

    bool isValid() override;
    static bool isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig);

    bool ResetToDefault();

protected:
    bool HandleConfigVersionConflict(const Version& configVersionFound) override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaReAppConfiguration)
};


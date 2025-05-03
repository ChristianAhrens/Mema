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

class AppConfiguration : public JUCEAppBasics::AppConfigurationBase
{

public:
    enum TagID
    {
        PROCESSORCONFIG,
        DEVCONFIG,
        EDITORCONFIG,
        PLUGINCONFIG,
        INPUTMUTES,
        OUTPUTMUTES,
        CROSSPOINTGAINS
    };
    static juce::String getTagName(TagID ID)
    {
        switch(ID)
        {
        case PROCESSORCONFIG:
            return "PROCESSORCONFIG";
        case DEVCONFIG:
            return "DEVICECONFIG";
        case EDITORCONFIG:
            return "EDITORCONFIG";
        case PLUGINCONFIG:
            return "PLUGINCONFIG";
        case INPUTMUTES:
            return "INPUTMUTES";
        case OUTPUTMUTES:
            return "OUTPUTMUTES";
        case CROSSPOINTGAINS:
            return "CROSSPOINTGAINS";
        default:
            return "INVALID";
        }
    };

    enum AttributeID
    {
        ENABLED,
        POST
    };
    static juce::String getAttributeName(AttributeID ID)
    {
        switch (ID)
        {
        case ENABLED:
            return "ENABLED";
        case POST:
            return "POST";
        default:
            return "-";
        }
    };

public:
    explicit AppConfiguration(const File &file);
    ~AppConfiguration() override;

    bool isValid() override;
    static bool isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig);

    bool ResetToDefault();

protected:
    bool HandleConfigVersionConflict(const Version& configVersionFound) override;

private:

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppConfiguration)
};

} // namespace SpaConBridge

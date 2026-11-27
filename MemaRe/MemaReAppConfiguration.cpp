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

#include "MemaReAppConfiguration.h"


MemaReAppConfiguration::MemaReAppConfiguration(const juce::File& file)
	: MemaClientAppConfigurationBase()
{
	InitializeBase(file, JUCEAppBasics::AppConfigurationBase::Version::FromString(Mema_CONFIG_VERSION));
}

MemaReAppConfiguration::~MemaReAppConfiguration()
{
}

bool MemaReAppConfiguration::isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig)
{
	return isValidClientConfiguration(xmlConfig, [](const juce::XmlElement& visuConfig) {
		auto outputPanningXmlElement = visuConfig.getChildByName(getTagName(TagID::CONTROLFORMAT));
		if (outputPanningXmlElement)
		{
			//validate
		}
		else
			return false;

		auto panningColourXmlElement = visuConfig.getChildByName(getTagName(TagID::CONTROLCOLOUR));
		if (panningColourXmlElement)
		{
			// validate
		}
		else
			return false;

		return true;
	});
}

juce::String MemaReAppConfiguration::getDefaultConfigurationXml()
{
	return juce::String(BinaryData::MemaReDefault_config, BinaryData::MemaReDefault_configSize);
}

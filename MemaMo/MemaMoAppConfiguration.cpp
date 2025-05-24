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

#include "MemaMoAppConfiguration.h"


MemaMoAppConfiguration::MemaMoAppConfiguration(const juce::File& file)
	: JUCEAppBasics::AppConfigurationBase()
{
	InitializeBase(file, JUCEAppBasics::AppConfigurationBase::Version::FromString(Mema_CONFIG_VERSION));
}

MemaMoAppConfiguration::~MemaMoAppConfiguration()
{
}

bool MemaMoAppConfiguration::isValid()
{
	return isValid(m_xml);
}

bool MemaMoAppConfiguration::isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig)
{
	if (!JUCEAppBasics::AppConfigurationBase::isValid(xmlConfig))
		return false;

	auto connectionConfigSectionElement = xmlConfig->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::CONNECTIONCONFIG));
	if (connectionConfigSectionElement)
	{
		auto serviceDescriptonXmlElement = connectionConfigSectionElement->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::SERVICEDESCRIPTION));
		if (serviceDescriptonXmlElement)
		{
			//validate
		}
		else
			return false;
	}
	else
		return false;

	auto visuConfigSectionElement = xmlConfig->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::VISUCONFIG));
	if (visuConfigSectionElement)
	{
		auto outputVisuXmlElement = visuConfigSectionElement->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::OUTPUTVISUTYPE));
		if (outputVisuXmlElement)
		{
			//validate
		}
		else
			return false;

		auto meteringColourXmlElement = visuConfigSectionElement->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::METERINGCOLOUR));
		if (meteringColourXmlElement)
		{
			// validate
		}
		else
			return false;

		auto lookAndFeelXmlElement = visuConfigSectionElement->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::LOOKANDFEEL));
		if (lookAndFeelXmlElement)
		{
			// validate
		}
		else
			return false;
	}
	else
		return false;

	return true;
}

bool MemaMoAppConfiguration::ResetToDefault()
{
	auto xmlConfig = juce::parseXML(juce::String(BinaryData::MemaMoDefault_config, BinaryData::MemaMoDefault_configSize));
	if (xmlConfig)
	{

		if (MemaMoAppConfiguration::isValid(xmlConfig))
		{

			SetFlushAndUpdateDisabled();
			if (resetConfigState(std::move(xmlConfig)))
			{
				ResetFlushAndUpdateDisabled();
				return true;
			}
			else
			{
				jassertfalse; // stop here when debugging, since invalid configurations often lead to endless debugging sessions until this simple explanation was found...
				ResetFlushAndUpdateDisabled();

				// ...and trigger generation of a valid config if not.
				triggerConfigurationDump();
			}
		}
		else
		{
			jassertfalse; // stop here when debugging, since invalid configurations often lead to endless debugging sessions until this simple explanation was found...

			// ...and trigger generation of a valid config if not.
			triggerConfigurationDump();
		}
	}
	else
	{
		jassertfalse; // stop here when debugging, since invalid configurations often lead to endless debugging sessions until this simple explanation was found...

		// ...and trigger generation of a valid config if not.
		triggerConfigurationDump();
	}

	return false;
}

bool MemaMoAppConfiguration::HandleConfigVersionConflict(const JUCEAppBasics::AppConfigurationBase::Version& configVersionFound)
{
	if (configVersionFound != JUCEAppBasics::AppConfigurationBase::Version::FromString(Mema_CONFIG_VERSION))
	{
		auto conflictTitle = "Incompatible configuration version";
		auto conflictInfo = "The configuration file version detected\ncannot be handled by this version of " + juce::JUCEApplication::getInstance()->getApplicationName();
#ifdef DEBUG
		conflictInfo << "\n(Found " + configVersionFound.ToString() + ", expected " + Mema_CONFIG_VERSION + ")";
#endif
		juce::AlertWindow::showOkCancelBox(juce::MessageBoxIconType::WarningIcon, conflictTitle, conflictInfo, "Reset to default", "Quit", nullptr, juce::ModalCallbackFunction::create([this](int result) {
			if (1 == result)
			{
				ResetToDefault();
			}
			else
			{
				juce::JUCEApplication::getInstance()->quit();
			}
		}));

		return false;
	}
	else
		return true;
}	


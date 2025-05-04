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

#include "AppConfiguration.h"

namespace Mema
{

AppConfiguration::AppConfiguration(const juce::File& file)
	: JUCEAppBasics::AppConfigurationBase()
{
	InitializeBase(file, JUCEAppBasics::AppConfigurationBase::Version::FromString(Mema_CONFIG_VERSION));
}

AppConfiguration::~AppConfiguration()
{
}

bool AppConfiguration::isValid()
{
	return isValid(m_xml);
}

bool AppConfiguration::isValid(const std::unique_ptr<juce::XmlElement>& xmlConfig)
{
	if (!JUCEAppBasics::AppConfigurationBase::isValid(xmlConfig))
		return false;

	auto editorCfgSectionElement = xmlConfig->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::UICONFIG));
	if (editorCfgSectionElement)
	{
		// validate
	}
	else
		return false;

	auto processorSectionElement = xmlConfig->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::PROCESSORCONFIG));
	if (processorSectionElement)
	{
		auto devSectionElement = processorSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::DEVCONFIG));
		if (devSectionElement)
		{
			// validate
		}
		else
			return false;

		auto plginSectionElement = processorSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::PLUGINCONFIG));
		if (plginSectionElement)
		{
			if (!plginSectionElement->hasAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::ENABLED)))
				return false;
			if (!plginSectionElement->hasAttribute(AppConfiguration::getAttributeName(AppConfiguration::AttributeID::POST)))
				return false;

			// validate
		}
		else
			return false;

		auto inputMutesSectionElement = processorSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::INPUTMUTES));
		if (inputMutesSectionElement)
		{
			// validate
		}
		else
			return false;

		auto crosspointGainsSectionElement = processorSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::CROSSPOINTGAINS));
		if (crosspointGainsSectionElement)
		{
			// validate
		}
		else
			return false;

		auto outputMutesSectionElement = processorSectionElement->getChildByName(AppConfiguration::getTagName(AppConfiguration::TagID::OUTPUTMUTES));
		if (outputMutesSectionElement)
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

bool AppConfiguration::ResetToDefault()
{
	auto xmlConfig = juce::parseXML(juce::String(BinaryData::Default_config, BinaryData::Default_configSize));
	if (xmlConfig)
	{

		if (Mema::AppConfiguration::isValid(xmlConfig))
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

bool AppConfiguration::HandleConfigVersionConflict(const JUCEAppBasics::AppConfigurationBase::Version& configVersionFound)
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


} // namespace Mema

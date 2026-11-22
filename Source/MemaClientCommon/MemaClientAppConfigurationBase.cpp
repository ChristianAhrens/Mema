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

#include "MemaClientAppConfigurationBase.h"


bool MemaClientAppConfigurationBase::isValid()
{
	return isConfigurationValid(m_xml);
}

bool MemaClientAppConfigurationBase::isValidClientConfiguration(const std::unique_ptr<juce::XmlElement>& xmlConfig,
	const std::function<bool(const juce::XmlElement& visuConfig)>& isVisuConfigValid)
{
	if (!JUCEAppBasics::AppConfigurationBase::isValid(xmlConfig))
		return false;

	auto connectionConfigSectionElement = xmlConfig->getChildByName(tagConnectionConfig);
	if (connectionConfigSectionElement)
	{
		auto serviceDescriptonXmlElement = connectionConfigSectionElement->getChildByName(tagServiceDescription);
		if (serviceDescriptonXmlElement)
		{
			//validate
		}
		else
			return false;
	}
	else
		return false;

	auto visuConfigSectionElement = xmlConfig->getChildByName(tagVisuConfig);
	if (visuConfigSectionElement)
	{
		if (isVisuConfigValid && !isVisuConfigValid(*visuConfigSectionElement))
			return false;

		auto lookAndFeelXmlElement = visuConfigSectionElement->getChildByName(tagLookAndFeel);
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

bool MemaClientAppConfigurationBase::ResetToDefault()
{
	auto xmlConfig = juce::parseXML(getDefaultConfigurationXml());
	if (xmlConfig)
	{

		if (isConfigurationValid(xmlConfig))
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

bool MemaClientAppConfigurationBase::HandleConfigVersionConflict(const JUCEAppBasics::AppConfigurationBase::Version& configVersionFound)
{
	if (configVersionFound != JUCEAppBasics::AppConfigurationBase::Version::FromString(Mema_CONFIG_VERSION))
	{
		auto conflictTitle = "Incompatible configuration version";
		auto conflictInfo = "The configuration file version detected\ncannot be handled by this version of " + juce::JUCEApplication::getInstance()->getApplicationName();
#ifdef DEBUG
		conflictInfo << "\n(Found " + configVersionFound.ToString() + ", expected " + Mema_CONFIG_VERSION + ")";
#endif
		juce::AlertWindow::showAsync(juce::MessageBoxOptions()
			.withIconType(juce::MessageBoxIconType::WarningIcon)
			.withTitle(conflictTitle)
			.withMessage(conflictInfo)
			.withButton("Reset to default", 1)
			.withButton("Quit", 0),
			[this](int result) {
				if (1 == result)
				{
					ResetToDefault();
				}
				else
				{
					juce::JUCEApplication::getInstance()->quit();
				}
		});

		return false;
	}
	else
		return true;
}	


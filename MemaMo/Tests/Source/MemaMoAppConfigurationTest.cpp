/* Copyright (c) 2026, Christian Ahrens
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

// Coverage for MemaMoAppConfiguration::isValid() (the static, XmlElement-taking
// overload) -- the gate that decides whether a loaded/received config file is
// complete enough to apply. As with Mema's MemaAppConfigurationTest.cpp, this
// deliberately does NOT construct a full MemaMoAppConfiguration instance (real
// file I/O + a background flush thread in AppConfigurationBase::InitializeBase()).
// The static isValid() overload only needs a live juce::JUCEApplication instance,
// for the application-name tag check in JUCEAppBasics::AppConfigurationBase::isValid().

#include <JuceHeader.h>
#include "../../MemaMoAppConfiguration.h"

namespace
{
    // See MemaAppConfigurationTest.cpp (Mema's Tests project) for why this is safe:
    // JUCEApplicationBase's constructor just registers the global instance pointer,
    // the message loop is never started, initialise()/shutdown() are never called.
    class DummyMemaMoApplication : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() override { return "Mema.Mo"; }
        const juce::String getApplicationVersion() override { return "0.0.0"; }
        void initialise (const juce::String&) override {}
        void shutdown() override {}
    };

    // Mirrors the structure MemaMoDefault.config and MemaMoComponent's config dump use.
    std::unique_ptr<juce::XmlElement> makeValidConfig()
    {
        auto root = std::make_unique<juce::XmlElement> ("Mema.Mo");

        auto* connectionConfig = root->createNewChildElement ("CONNECTIONCONFIG");
        connectionConfig->createNewChildElement ("SERVICEDESCRIPTION");

        auto* visuConfig = root->createNewChildElement ("VISUCONFIG");
        visuConfig->createNewChildElement ("OUTPUTVISUTYPE");
        visuConfig->createNewChildElement ("METERINGCOLOUR");
        visuConfig->createNewChildElement ("LOOKANDFEEL");

        return root;
    }
}

class MemaMoAppConfigurationIsValidTest : public juce::UnitTest
{
public:
    MemaMoAppConfigurationIsValidTest() : juce::UnitTest ("MemaMoAppConfiguration::isValid", "Mema.Mo") {}

    void runTest() override
    {
        DummyMemaMoApplication dummyApp;

        beginTest ("A fully populated config tree is valid");
        {
            auto config = makeValidConfig();
            expect (MemaMoAppConfiguration::isValid (config));
        }

        beginTest ("A null config is invalid");
        {
            std::unique_ptr<juce::XmlElement> nullConfig;
            expect (! MemaMoAppConfiguration::isValid (nullConfig));
        }

        beginTest ("Wrong root tag name is invalid");
        {
            auto config = std::make_unique<juce::XmlElement> ("NotMemaMo");
            expect (! MemaMoAppConfiguration::isValid (config));
        }

        beginTest ("Missing CONNECTIONCONFIG is invalid");
        {
            auto config = makeValidConfig();
            config->removeChildElement (config->getChildByName ("CONNECTIONCONFIG"), true);
            expect (! MemaMoAppConfiguration::isValid (config));
        }

        beginTest ("Missing SERVICEDESCRIPTION within CONNECTIONCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* connectionConfig = config->getChildByName ("CONNECTIONCONFIG");
            connectionConfig->removeChildElement (connectionConfig->getChildByName ("SERVICEDESCRIPTION"), true);
            expect (! MemaMoAppConfiguration::isValid (config));
        }

        beginTest ("Missing VISUCONFIG is invalid");
        {
            auto config = makeValidConfig();
            config->removeChildElement (config->getChildByName ("VISUCONFIG"), true);
            expect (! MemaMoAppConfiguration::isValid (config));
        }

        beginTest ("Missing OUTPUTVISUTYPE within VISUCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* visuConfig = config->getChildByName ("VISUCONFIG");
            visuConfig->removeChildElement (visuConfig->getChildByName ("OUTPUTVISUTYPE"), true);
            expect (! MemaMoAppConfiguration::isValid (config));
        }

        beginTest ("Missing METERINGCOLOUR within VISUCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* visuConfig = config->getChildByName ("VISUCONFIG");
            visuConfig->removeChildElement (visuConfig->getChildByName ("METERINGCOLOUR"), true);
            expect (! MemaMoAppConfiguration::isValid (config));
        }

        beginTest ("Missing LOOKANDFEEL within VISUCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* visuConfig = config->getChildByName ("VISUCONFIG");
            visuConfig->removeChildElement (visuConfig->getChildByName ("LOOKANDFEEL"), true);
            expect (! MemaMoAppConfiguration::isValid (config));
        }
    }
};

static MemaMoAppConfigurationIsValidTest memaMoAppConfigurationIsValidTest;

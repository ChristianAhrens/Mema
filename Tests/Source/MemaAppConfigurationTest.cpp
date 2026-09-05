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

// Coverage for MemaAppConfiguration::isValid() (the static, XmlElement-taking
// overload) -- the gate that decides whether a loaded/received config file is
// complete enough to apply. Deliberately does NOT construct a full
// MemaAppConfiguration instance: its constructor does real file I/O and starts a
// background flush thread (see AppConfigurationBase::InitializeBase()), which is
// both unnecessary for testing the pure validation logic and adds real risk of an
// untested lifecycle interaction in a plain console-app test harness. The static
// isValid() overload only needs a live juce::JUCEApplication instance (for the
// application-name tag check in JUCEAppBasics::AppConfigurationBase::isValid()),
// which is provided locally and torn down at the end of the test.

#include <JuceHeader.h>
#include <MemaAppConfiguration.h>

using namespace Mema;

namespace
{
    // MemaAppConfiguration::isValid() calls (via AppConfigurationBase::isValid())
    // juce::JUCEApplication::getInstance()->getApplicationName(), which is null outside a
    // real JUCEApplication. Constructing this subclass is enough to satisfy that --
    // JUCEApplicationBase's constructor just registers the global instance pointer; the
    // message loop is never started and initialise()/shutdown() are never invoked.
    class DummyMemaApplication : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() override { return "Mema"; }
        const juce::String getApplicationVersion() override { return "0.0.0"; }
        void initialise (const juce::String&) override {}
        void shutdown() override {}
    };

    // Mirrors the structure MemaProcessor::createStateXml()/MemaUIComponent build up in the
    // real app, and what MemaDefault.config contains.
    std::unique_ptr<juce::XmlElement> makeValidConfig()
    {
        auto root = std::make_unique<juce::XmlElement> ("Mema");
        root->createNewChildElement ("UICONFIG");

        auto* processorConfig = root->createNewChildElement ("PROCESSORCONFIG");
        processorConfig->createNewChildElement ("DEVICECONFIG");
        auto* pluginConfig = processorConfig->createNewChildElement ("PLUGINCONFIG");
        pluginConfig->setAttribute ("ENABLED", 0);
        pluginConfig->setAttribute ("POST", 0);
        processorConfig->createNewChildElement ("INPUTMUTES");
        processorConfig->createNewChildElement ("OUTPUTMUTES");
        processorConfig->createNewChildElement ("CROSSPOINTGAINS");

        return root;
    }
}

class MemaAppConfigurationIsValidTest : public juce::UnitTest
{
public:
    MemaAppConfigurationIsValidTest() : juce::UnitTest ("MemaAppConfiguration::isValid", "Mema") {}

    void runTest() override
    {
        DummyMemaApplication dummyApp;

        beginTest ("A fully populated config tree is valid");
        {
            auto config = makeValidConfig();
            expect (MemaAppConfiguration::isValid (config));
        }

        beginTest ("A null config is invalid");
        {
            std::unique_ptr<juce::XmlElement> nullConfig;
            expect (! MemaAppConfiguration::isValid (nullConfig));
        }

        beginTest ("Wrong root tag name is invalid");
        {
            auto config = std::make_unique<juce::XmlElement> ("NotMema");
            expect (! MemaAppConfiguration::isValid (config));
        }

        beginTest ("Missing UICONFIG is invalid");
        {
            auto config = makeValidConfig();
            config->removeChildElement (config->getChildByName ("UICONFIG"), true);
            expect (! MemaAppConfiguration::isValid (config));
        }

        beginTest ("Missing PROCESSORCONFIG is invalid");
        {
            auto config = makeValidConfig();
            config->removeChildElement (config->getChildByName ("PROCESSORCONFIG"), true);
            expect (! MemaAppConfiguration::isValid (config));
        }

        beginTest ("Missing DEVICECONFIG within PROCESSORCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* processorConfig = config->getChildByName ("PROCESSORCONFIG");
            processorConfig->removeChildElement (processorConfig->getChildByName ("DEVICECONFIG"), true);
            expect (! MemaAppConfiguration::isValid (config));
        }

        beginTest ("PLUGINCONFIG missing the ENABLED attribute is invalid");
        {
            auto config = makeValidConfig();
            auto* pluginConfig = config->getChildByName ("PROCESSORCONFIG")->getChildByName ("PLUGINCONFIG");
            pluginConfig->removeAttribute ("ENABLED");
            expect (! MemaAppConfiguration::isValid (config));
        }

        beginTest ("PLUGINCONFIG missing the POST attribute is invalid");
        {
            auto config = makeValidConfig();
            auto* pluginConfig = config->getChildByName ("PROCESSORCONFIG")->getChildByName ("PLUGINCONFIG");
            pluginConfig->removeAttribute ("POST");
            expect (! MemaAppConfiguration::isValid (config));
        }

        beginTest ("Missing INPUTMUTES within PROCESSORCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* processorConfig = config->getChildByName ("PROCESSORCONFIG");
            processorConfig->removeChildElement (processorConfig->getChildByName ("INPUTMUTES"), true);
            expect (! MemaAppConfiguration::isValid (config));
        }

        beginTest ("Missing OUTPUTMUTES within PROCESSORCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* processorConfig = config->getChildByName ("PROCESSORCONFIG");
            processorConfig->removeChildElement (processorConfig->getChildByName ("OUTPUTMUTES"), true);
            expect (! MemaAppConfiguration::isValid (config));
        }

        beginTest ("Missing CROSSPOINTGAINS within PROCESSORCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* processorConfig = config->getChildByName ("PROCESSORCONFIG");
            processorConfig->removeChildElement (processorConfig->getChildByName ("CROSSPOINTGAINS"), true);
            expect (! MemaAppConfiguration::isValid (config));
        }
    }
};

static MemaAppConfigurationIsValidTest memaAppConfigurationIsValidTest;

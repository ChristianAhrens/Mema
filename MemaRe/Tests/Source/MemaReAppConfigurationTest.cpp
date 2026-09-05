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

// Coverage for MemaReAppConfiguration::isValid() (the static, XmlElement-taking
// overload). As with Mema/Mema.Mo's *AppConfigurationTest.cpp, this deliberately
// does NOT construct a full MemaReAppConfiguration instance (real file I/O + a
// background flush thread in AppConfigurationBase::InitializeBase()).
//
// IMPORTANT: the XML built by makeValidConfig() below uses the tag names
// MemaReAppConfiguration.h's own TagID enum declares (CONTROLFORMAT, CONTROLCOLOUR)
// -- that is genuinely what isValid() checks for, and is what's being tested here.
// It does NOT match the tag names actually present in the shipped
// Resources/MemaReDefault.config, which uses <OUTPUTPANNINGTYPE/> and
// <PANNINGCOLOUR/> instead of <CONTROLFORMAT/> and <CONTROLCOLOUR/>. That
// mismatch means MemaReAppConfiguration::isValid() rejects the app's own bundled
// default config, so ResetToDefault() currently always falls through to its
// "invalid configuration" branch instead of ever successfully applying the
// shipped default. This is a real, separate bug -- reported alongside this test
// batch rather than fixed here. Once fixed (by renaming either the code's TagID
// strings or the config file's tags to match), it would be worth adding a test
// here that parses the actual Resources/MemaReDefault.config content and asserts
// isValid() accepts it, as a permanent regression guard for this exact mismatch.

#include <JuceHeader.h>
#include "../../MemaReAppConfiguration.h"

namespace
{
    // See MemaAppConfigurationTest.cpp (Mema's Tests project) for why this is safe.
    class DummyMemaReApplication : public juce::JUCEApplication
    {
    public:
        const juce::String getApplicationName() override { return "Mema.Re"; }
        const juce::String getApplicationVersion() override { return "0.0.0"; }
        void initialise (const juce::String&) override {}
        void shutdown() override {}
    };

    // Matches MemaReAppConfiguration.h's own TagID tag names -- see file header note
    // above regarding the mismatch with the actual shipped MemaReDefault.config.
    std::unique_ptr<juce::XmlElement> makeValidConfig()
    {
        auto root = std::make_unique<juce::XmlElement> ("Mema.Re");

        auto* connectionConfig = root->createNewChildElement ("CONNECTIONCONFIG");
        connectionConfig->createNewChildElement ("SERVICEDESCRIPTION");

        auto* visuConfig = root->createNewChildElement ("VISUCONFIG");
        visuConfig->createNewChildElement ("CONTROLFORMAT");
        visuConfig->createNewChildElement ("CONTROLCOLOUR");
        visuConfig->createNewChildElement ("LOOKANDFEEL");

        return root;
    }
}

class MemaReAppConfigurationIsValidTest : public juce::UnitTest
{
public:
    MemaReAppConfigurationIsValidTest() : juce::UnitTest ("MemaReAppConfiguration::isValid", "Mema.Re") {}

    void runTest() override
    {
        DummyMemaReApplication dummyApp;

        beginTest ("A fully populated config tree (per the code's own tag names) is valid");
        {
            auto config = makeValidConfig();
            expect (MemaReAppConfiguration::isValid (config));
        }

        beginTest ("A null config is invalid");
        {
            std::unique_ptr<juce::XmlElement> nullConfig;
            expect (! MemaReAppConfiguration::isValid (nullConfig));
        }

        beginTest ("Wrong root tag name is invalid");
        {
            auto config = std::make_unique<juce::XmlElement> ("NotMemaRe");
            expect (! MemaReAppConfiguration::isValid (config));
        }

        beginTest ("Missing CONNECTIONCONFIG is invalid");
        {
            auto config = makeValidConfig();
            config->removeChildElement (config->getChildByName ("CONNECTIONCONFIG"), true);
            expect (! MemaReAppConfiguration::isValid (config));
        }

        beginTest ("Missing SERVICEDESCRIPTION within CONNECTIONCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* connectionConfig = config->getChildByName ("CONNECTIONCONFIG");
            connectionConfig->removeChildElement (connectionConfig->getChildByName ("SERVICEDESCRIPTION"), true);
            expect (! MemaReAppConfiguration::isValid (config));
        }

        beginTest ("Missing VISUCONFIG is invalid");
        {
            auto config = makeValidConfig();
            config->removeChildElement (config->getChildByName ("VISUCONFIG"), true);
            expect (! MemaReAppConfiguration::isValid (config));
        }

        beginTest ("Missing CONTROLFORMAT within VISUCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* visuConfig = config->getChildByName ("VISUCONFIG");
            visuConfig->removeChildElement (visuConfig->getChildByName ("CONTROLFORMAT"), true);
            expect (! MemaReAppConfiguration::isValid (config));
        }

        beginTest ("Missing CONTROLCOLOUR within VISUCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* visuConfig = config->getChildByName ("VISUCONFIG");
            visuConfig->removeChildElement (visuConfig->getChildByName ("CONTROLCOLOUR"), true);
            expect (! MemaReAppConfiguration::isValid (config));
        }

        beginTest ("Missing LOOKANDFEEL within VISUCONFIG is invalid");
        {
            auto config = makeValidConfig();
            auto* visuConfig = config->getChildByName ("VISUCONFIG");
            visuConfig->removeChildElement (visuConfig->getChildByName ("LOOKANDFEEL"), true);
            expect (! MemaReAppConfiguration::isValid (config));
        }

        beginTest ("The actual shipped default config's tag names do not satisfy isValid() [known bug]");
        {
            // Documents the mismatch described in the file header: the real
            // MemaReDefault.config uses OUTPUTPANNINGTYPE/PANNINGCOLOUR, which the
            // current isValid() does not look for at all.
            auto config = makeValidConfig();
            auto* visuConfig = config->getChildByName ("VISUCONFIG");
            visuConfig->removeChildElement (visuConfig->getChildByName ("CONTROLFORMAT"), true);
            visuConfig->createNewChildElement ("OUTPUTPANNINGTYPE");
            visuConfig->removeChildElement (visuConfig->getChildByName ("CONTROLCOLOUR"), true);
            visuConfig->createNewChildElement ("PANNINGCOLOUR");

            expect (! MemaReAppConfiguration::isValid (config), "confirms today's isValid() rejects the shipped config's actual tag names");
        }
    }
};

static MemaReAppConfigurationIsValidTest memaReAppConfigurationIsValidTest;

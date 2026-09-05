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

// Coverage for Mema::PluginParameterInfo -- the plain-data descriptor used to
// mirror a hosted plugin's parameters to Mema.Re (see PluginParameterInfosMessage
// in MemaMessages.h). Exercises the string serialisation used internally
// (toString()/initializeFromString()) and the extraction from a real
// juce::AudioProcessorParameter (initializeFromAudioProcessorParameter()).

#include <JuceHeader.h>
#include <MemaProcessor/MemaPluginParameterInfo.h>

using namespace Mema;

class PluginParameterInfoStringRoundTripTest : public juce::UnitTest
{
public:
    PluginParameterInfoStringRoundTripTest() : juce::UnitTest ("PluginParameterInfo string round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Continuous parameter (no step names) round-trips through toString()/fromString()");
        {
            PluginParameterInfo info;
            info.index = 3;
            info.id = "gain";
            info.name = "Gain";
            info.defaultValue = 0.5f;
            info.currentValue = 0.75f;
            info.label = "dB";
            info.isAutomatable = true;
            info.isRemoteControllable = false;
            info.category = juce::AudioProcessorParameter::genericParameter;
            info.minValue = 0.0f;
            info.maxValue = 1.0f;
            info.stepSize = 0.0f;
            info.isDiscrete = false;
            info.type = ParameterControlType::Continuous;
            info.stepCount = 0;

            auto roundTripped = PluginParameterInfo::fromString (info.toString());
            expect (roundTripped == info, "round-tripped PluginParameterInfo should equal the original");
        }

        beginTest ("Discrete parameter with step names round-trips, including step name list");
        {
            PluginParameterInfo info;
            info.index = 1;
            info.id = "mode";
            info.name = "Mode";
            info.defaultValue = 0.0f;
            info.currentValue = 0.5f;
            info.label = "";
            info.isAutomatable = false;
            info.isRemoteControllable = true;
            info.category = juce::AudioProcessorParameter::genericParameter;
            info.minValue = 0.0f;
            info.maxValue = 2.0f;
            info.stepSize = 1.0f;
            info.isDiscrete = true;
            info.type = ParameterControlType::Discrete;
            info.stepCount = 3;
            info.stepNames = { "Low", "Mid", "High" };

            auto roundTripped = PluginParameterInfo::fromString (info.toString());
            expect (roundTripped == info, "round-tripped discrete PluginParameterInfo should equal the original");
            expectEquals ((int) roundTripped.stepNames.size(), 3);
            expectEquals (juce::String (roundTripped.stepNames[1]), juce::String ("Mid"));
        }

        beginTest ("initializeFromString() rejects a malformed (wrong field count) string");
        {
            PluginParameterInfo info;
            auto ok = info.initializeFromString ("not;enough;fields");
            expect (! ok, "a string with fewer than 16 fields should be rejected");
        }

        beginTest ("A ';' inside a field breaks the round-trip [documents current limitation]");
        {
            // toString()/initializeFromString() use ';' as the field separator with no escaping.
            // A plugin parameter name/id/label containing a literal ';' produces more than 16
            // tokens on parse, so initializeFromString() returns false and the round-trip fails
            // outright (not a crash, but a silent data-loss risk for unusual plugin metadata).
            PluginParameterInfo info;
            info.index = 0;
            info.id = "p";
            info.name = "Cutoff; Resonance";
            info.defaultValue = 0.0f;
            info.currentValue = 0.0f;
            info.label = "";
            info.category = juce::AudioProcessorParameter::genericParameter;
            info.minValue = 0.0f;
            info.maxValue = 1.0f;
            info.stepSize = 0.0f;
            info.type = ParameterControlType::Continuous;
            info.stepCount = 0;

            PluginParameterInfo roundTripped;
            auto ok = roundTripped.initializeFromString (info.toString());
            expect (! ok, "a ';' embedded in a field is currently indistinguishable from the field separator");
        }
    }
};

static PluginParameterInfoStringRoundTripTest pluginParameterInfoStringRoundTripTest;

//==============================================================================
class PluginParameterInfoFromAudioProcessorParameterTest : public juce::UnitTest
{
public:
    PluginParameterInfoFromAudioProcessorParameterTest() : juce::UnitTest ("PluginParameterInfo from AudioProcessorParameter", "Mema") {}

    void runTest() override
    {
        beginTest ("Continuous ranged parameter is extracted correctly");
        {
            juce::AudioParameterFloat param ("gain", "Gain", juce::NormalisableRange<float> (0.0f, 1.0f, 0.0f), 0.5f);

            auto info = PluginParameterInfo::fromAudioProcessorParameter (param);

            expectEquals (info.name, juce::String ("Gain"));
            expectEquals (info.id, juce::String ("gain"));
            expectEquals (info.minValue, 0.0f);
            expectEquals (info.maxValue, 1.0f);
            expectEquals (info.stepSize, 0.0f);
            expect (! info.isDiscrete, "a NormalisableRange with interval 0 is continuous, not discrete");
            // an unattached parameter (never added to an AudioProcessor) has no assigned index
            expectEquals (info.index, -1);
            // a freshly constructed parameter's current value equals its default
            expectWithinAbsoluteError (info.currentValue, info.defaultValue, 1.0e-6f);
        }

        beginTest ("Discrete ranged parameter (non-zero interval) is flagged as discrete");
        {
            juce::AudioParameterFloat param ("steps", "Steps", juce::NormalisableRange<float> (0.0f, 10.0f, 1.0f), 5.0f);

            auto info = PluginParameterInfo::fromAudioProcessorParameter (param);

            expectEquals (info.minValue, 0.0f);
            expectEquals (info.maxValue, 10.0f);
            expectEquals (info.stepSize, 1.0f);
            expect (info.isDiscrete, "a NormalisableRange with a non-zero interval is discrete");
        }

        beginTest ("Bool parameter (ranged, [0,1]) is extracted correctly");
        {
            juce::AudioParameterBool param ("bypass", "Bypass", false);

            auto info = PluginParameterInfo::fromAudioProcessorParameter (param);

            expectEquals (info.id, juce::String ("bypass"));
            expectEquals (info.name, juce::String ("Bypass"));
            expectEquals (info.minValue, 0.0f);
            expectEquals (info.maxValue, 1.0f);
        }

        beginTest ("parametersToInfos() preserves order and count for several parameters");
        {
            juce::AudioParameterFloat p1 ("a", "A", juce::NormalisableRange<float> (0.0f, 1.0f, 0.0f), 0.0f);
            juce::AudioParameterFloat p2 ("b", "B", juce::NormalisableRange<float> (0.0f, 1.0f, 0.0f), 1.0f);
            juce::Array<juce::AudioProcessorParameter*> params;
            params.add (&p1);
            params.add (&p2);

            auto infos = PluginParameterInfo::parametersToInfos (params);

            expectEquals ((int) infos.size(), 2);
            expectEquals (infos[0].id, juce::String ("a"));
            expectEquals (infos[1].id, juce::String ("b"));
        }
    }
};

static PluginParameterInfoFromAudioProcessorParameterTest pluginParameterInfoFromAudioProcessorParameterTest;

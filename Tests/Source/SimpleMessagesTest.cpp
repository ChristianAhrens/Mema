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

// Round-trip coverage for the smaller fixed-size Mema::SerializableMessage
// subclasses: ReinitIOCountMessage and AnalyzerParametersMessage. Exercises the
// same production serialise/deserialise path as ControlParametersMessageTest.cpp.

#include <JuceHeader.h>
#include <MemaProcessor/MemaMessages.h>

using namespace Mema;

class ReinitIOCountMessageTest : public juce::UnitTest
{
public:
    ReinitIOCountMessageTest() : juce::UnitTest ("ReinitIOCountMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Input/output counts round-trip");

        ReinitIOCountMessage original (3, 12);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        expect (deserialized != nullptr, "initFromMemoryBlock() should recognise a ReinitIOCount frame");

        auto* riocm = dynamic_cast<ReinitIOCountMessage*> (deserialized);
        expect (riocm != nullptr, "initFromMemoryBlock() should construct a ReinitIOCountMessage");

        if (riocm != nullptr)
        {
            expectEquals (int (riocm->getInputCount()), 3);
            expectEquals (int (riocm->getOutputCount()), 12);
        }

        SerializableMessage::freeMessageData (deserialized);
    }
};

static ReinitIOCountMessageTest reinitIOCountMessageTest;

//==============================================================================
class AnalyzerParametersMessageTest : public juce::UnitTest
{
public:
    AnalyzerParametersMessageTest() : juce::UnitTest ("AnalyzerParametersMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Sample rate and block size round-trip");

        AnalyzerParametersMessage original (48000, 512);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        expect (deserialized != nullptr, "initFromMemoryBlock() should recognise an AnalyzerParameters frame");

        auto* apm = dynamic_cast<AnalyzerParametersMessage*> (deserialized);
        expect (apm != nullptr, "initFromMemoryBlock() should construct an AnalyzerParametersMessage");

        if (apm != nullptr)
        {
            // NOTE: the wire payload is uint16, so values above 65535 (e.g. some high
            // sample rates) would already be truncated by the constructor itself, before
            // serialisation even happens -- not a round-trip bug, but worth knowing about
            // if this message is ever used with sample rates that don't fit uint16.
            expectEquals (apm->getSampleRate(), 48000);
            expectEquals (apm->getMaximumExpectedSamplesPerBlock(), 512);
        }

        SerializableMessage::freeMessageData (deserialized);
    }
};

static AnalyzerParametersMessageTest analyzerParametersMessageTest;

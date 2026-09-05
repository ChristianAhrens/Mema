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

// Round-trip coverage for Mema::ControlParametersMessage -- the message used to
// exchange the full routing-matrix state (mutes, crosspoint enables, crosspoint
// gains) between Mema and Mema.Re. Exercises the exact serialise/deserialise path
// used in production (SerializableMessage::getSerializedMessage() /
// ::initFromMemoryBlock()), for the message shapes that Mema and Mema.Re actually
// construct: empty (mute-only updates), a single changed crosspoint (Mema's
// per-change echo), a single input with several changed outputs at once (Mema.Re's
// panning-mode update, see PanningControlComponent::processOutputDistances()), a
// full dense N x M matrix snapshot (Mema's on-connect sync), and a "ragged" map
// with several inputs each carrying a different number of outputs.
//
// The ragged-map case is a regression test: createSerializedContent() used to
// compute the crosspoint sections' wire element counts as
// `m_crosspointStates.size() * m_crosspointStates.begin()->second.size()`
// (and the equivalent for crosspointValues) instead of summing each input's
// actual inner-map size. That was only correct when every input had the same
// number of entries, which happened to be true for every shape any current call
// site constructs -- but a genuinely ragged map made the written count disagree
// with the number of (in, out, value) triples actually written, desynchronising
// every field read after it on the receiving end. Fixed in createSerializedContent()
// to sum actual per-input sizes; this test guards against a regression.

#include <JuceHeader.h>
#include <MemaProcessor/MemaMessages.h>

using namespace Mema;

class ControlParametersMessageTest : public juce::UnitTest
{
public:
    ControlParametersMessageTest() : juce::UnitTest ("ControlParametersMessage round-trip", "Mema") {}

    void runTest() override
    {
        beginTest ("Empty message round-trips (pure mute-only updates send empty crosspoint maps)");
        {
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }

        beginTest ("Mute states round-trip");
        {
            std::map<std::uint16_t, bool> inputMuteStates { { 1, true }, { 2, false }, { 3, true } };
            std::map<std::uint16_t, bool> outputMuteStates { { 1, false }, { 4, true } };
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }

        beginTest ("Single crosspoint change round-trips (Mema's per-change echo to other clients)");
        {
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
            crosspointValues[2][5] = 0.75f;

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }

        beginTest ("Single input, several outputs round-trips (Mema.Re panning-mode update)");
        {
            // Mirrors PanningControlComponent::processOutputDistances(): one input channel
            // carries gains for several outputs at once, since the crossfade around a
            // hard-pan position can touch more than one output simultaneously.
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
            crosspointValues[3][1] = 0.1f;
            crosspointValues[3][2] = 0.9f;
            crosspointValues[3][6] = 0.3f;

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }

        beginTest ("Full dense NxM matrix snapshot round-trips (Mema's on-connect sync)");
        {
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
            for (std::uint16_t in = 1; in <= 4; ++in)
            {
                for (std::uint16_t out = 1; out <= 6; ++out)
                {
                    crosspointStates[in][out] = (in == out);
                    crosspointValues[in][out] = (in == out) ? 1.0f : 0.0f;
                }
            }

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }

        beginTest ("Ragged map (inputs with differing output counts) round-trips [regression]");
        {
            // Regression test for the fixed createSerializedContent() count computation: three
            // inputs, each with a different number of outputs. The old
            // `size() * begin()->second.size()` formula would compute 3 * 1 = 3 for
            // crosspointValuesCount here, even though 6 (in, out, value) triples are actually
            // written -- desynchronising every field parsed after this section on receipt.
            std::map<std::uint16_t, bool> inputMuteStates;
            std::map<std::uint16_t, bool> outputMuteStates;
            std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
            std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
            crosspointStates[1][1] = true;
            crosspointStates[2][1] = true;
            crosspointStates[2][2] = false;
            crosspointStates[3][1] = true;
            crosspointStates[3][2] = true;
            crosspointStates[3][3] = true;
            crosspointValues[1][1] = 1.0f;
            crosspointValues[2][1] = 0.5f;
            crosspointValues[2][2] = 0.25f;
            crosspointValues[3][1] = 0.1f;
            crosspointValues[3][2] = 0.2f;
            crosspointValues[3][3] = 0.3f;

            roundTripAndCompare (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        }
    }

private:
    void roundTripAndCompare (const std::map<std::uint16_t, bool>& inputMuteStates,
                               const std::map<std::uint16_t, bool>& outputMuteStates,
                               const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates,
                               const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues)
    {
        ControlParametersMessage original (inputMuteStates, outputMuteStates, crosspointStates, crosspointValues);
        auto blob = original.getSerializedMessage();

        auto* deserialized = SerializableMessage::initFromMemoryBlock (blob);
        expect (deserialized != nullptr, "initFromMemoryBlock() should recognise a ControlParameters frame");

        auto* cpm = dynamic_cast<ControlParametersMessage*> (deserialized);
        expect (cpm != nullptr, "initFromMemoryBlock() should construct a ControlParametersMessage");

        if (cpm != nullptr)
        {
            expect (cpm->getInputMuteStates() == inputMuteStates, "input mute states should survive the round-trip");
            expect (cpm->getOutputMuteStates() == outputMuteStates, "output mute states should survive the round-trip");
            expect (cpm->getCrosspointStates() == crosspointStates, "crosspoint enable states should survive the round-trip");
            expect (cpm->getCrosspointValues() == crosspointValues, "crosspoint gain values should survive the round-trip");
        }

        SerializableMessage::freeMessageData (deserialized);
    }
};

static ControlParametersMessageTest controlParametersMessageTest;

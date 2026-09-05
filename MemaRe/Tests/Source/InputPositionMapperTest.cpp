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

// Coverage for Mema::InputPositionMapper -- the core panning algorithm behind
// Mema.Re's 2D-field control: given an input's (x, y) position and a sharpness
// setting, it computes a per-output gain for every configured output position.
// This is pure math (no juce::Component involved), driven entirely through two
// std::function hooks (getAngleForChannelType, onInputPositionMapped), so it's
// directly unit-testable.
//
// The two synthetic output positions used below (Left at -90 degrees, Right at
// +90 degrees, i.e. (-1,0) and (1,0)) are chosen to make the expected proximity/
// level values exact and hand-verifiable, matching the documented algorithm in
// InputPositionMapper.h -- not because they're how any real Mema speaker layout
// is actually angled.

#include <JuceHeader.h>
#include <MemaClientCommon/InputPositionMapper.h>

using namespace Mema;

namespace
{
    float angleForTestChannel (const juce::AudioChannelSet::ChannelType& channelType)
    {
        // Left at -90 degrees -> (sin(-90), -cos(-90)) = (-1, 0)
        // Right at +90 degrees -> (sin(90), -cos(90)) = (1, 0)
        if (channelType == juce::AudioChannelSet::left)
            return -90.0f;
        if (channelType == juce::AudioChannelSet::right)
            return 90.0f;
        jassertfalse;
        return 0.0f;
    }
}

class InputPositionMapperTest : public juce::UnitTest
{
public:
    InputPositionMapperTest() : juce::UnitTest ("InputPositionMapper", "Mema.Re") {}

    void runTest() override
    {
        beginTest ("Without both hooks set, mapInputPosition() is a safe no-op");
        {
            InputPositionMapper mapper;
            mapper.setOutputIncludePositions ({ juce::AudioChannelSet::left });
            // neither hook set -- should not crash, callback simply never fires
            mapper.mapInputPosition (1, { 0.0f, 0.0f }, 0.0f);
        }

        beginTest ("Input exactly at a single output's position yields full level for that output");
        {
            InputPositionMapper mapper;
            mapper.getAngleForChannelType = angleForTestChannel;
            mapper.setOutputIncludePositions ({ juce::AudioChannelSet::left });

            std::map<juce::AudioChannelSet::ChannelType, float> result;
            mapper.onInputPositionMapped = [&] (std::uint16_t, const std::map<juce::AudioChannelSet::ChannelType, float>& dists)
            {
                result = dists;
            };

            mapper.mapInputPosition (1, { -1.0f, 0.0f }, 0.0f); // exactly on Left's position, sharpness 0

            expectWithinAbsoluteError (result[juce::AudioChannelSet::left], 1.0f, 1.0e-5f);
        }

        beginTest ("Centred input with zero sharpness splits level equally between two symmetric outputs");
        {
            InputPositionMapper mapper;
            mapper.getAngleForChannelType = angleForTestChannel;
            mapper.setOutputIncludePositions ({ juce::AudioChannelSet::left, juce::AudioChannelSet::right });

            std::map<juce::AudioChannelSet::ChannelType, float> result;
            mapper.onInputPositionMapped = [&] (std::uint16_t, const std::map<juce::AudioChannelSet::ChannelType, float>& dists)
            {
                result = dists;
            };

            mapper.mapInputPosition (1, { 0.0f, 0.0f }, 0.0f); // field centre, no sharpness

            // distance from centre to either output is 1.0 on the unit circle -> proximity 0.5,
            // and with sharpness 0 the exponent is 1, so the level is exactly 0.5 for both.
            expectWithinAbsoluteError (result[juce::AudioChannelSet::left], 0.5f, 1.0e-5f);
            expectWithinAbsoluteError (result[juce::AudioChannelSet::right], 0.5f, 1.0e-5f);
        }

        beginTest ("Maximum sharpness at the field edge gives exact single-output isolation");
        {
            InputPositionMapper mapper;
            mapper.getAngleForChannelType = angleForTestChannel;
            mapper.setOutputIncludePositions ({ juce::AudioChannelSet::left, juce::AudioChannelSet::right });

            std::map<juce::AudioChannelSet::ChannelType, float> result;
            mapper.onInputPositionMapped = [&] (std::uint16_t, const std::map<juce::AudioChannelSet::ChannelType, float>& dists)
            {
                result = dists;
            };

            // exactly at Left's position (radius 1.0) with sharpness 1.0 -> hardPanBlend reaches
            // exactly 1.0, so the result must be the *exact* hard values, not just close to them.
            mapper.mapInputPosition (1, { -1.0f, 0.0f }, 1.0f);

            expectEquals (result[juce::AudioChannelSet::left], 1.0f);
            expectEquals (result[juce::AudioChannelSet::right], 0.0f);
        }

        beginTest ("Maximum sharpness at the field centre does NOT force single-output isolation");
        {
            // Documented behaviour: hardPanBlend is gated by radius, so even sharpness == 1.0
            // still blends across outputs when the input sits at the centre (radius 0).
            InputPositionMapper mapper;
            mapper.getAngleForChannelType = angleForTestChannel;
            mapper.setOutputIncludePositions ({ juce::AudioChannelSet::left, juce::AudioChannelSet::right });

            std::map<juce::AudioChannelSet::ChannelType, float> result;
            mapper.onInputPositionMapped = [&] (std::uint16_t, const std::map<juce::AudioChannelSet::ChannelType, float>& dists)
            {
                result = dists;
            };

            mapper.mapInputPosition (1, { 0.0f, 0.0f }, 1.0f); // centre, max sharpness

            // proximity 0.5 for both, exponent jmap(1, 1, 5) = 5 -> 0.5^5 = 0.03125, and
            // hardPanBlend == 0 here (radius 0), so the smooth value must survive untouched.
            expectWithinAbsoluteError (result[juce::AudioChannelSet::left], 0.03125f, 1.0e-5f);
            expectWithinAbsoluteError (result[juce::AudioChannelSet::right], 0.03125f, 1.0e-5f);
            expect (result[juce::AudioChannelSet::left] < 1.0f, "centre position must never reach exact single-output isolation, even at max sharpness");
        }

        beginTest ("Outputs in the ignore list are always forced to zero");
        {
            InputPositionMapper mapper;
            mapper.getAngleForChannelType = angleForTestChannel;
            mapper.setOutputIncludePositions ({ juce::AudioChannelSet::left });
            mapper.setOutputIgnorePositions ({ juce::AudioChannelSet::right });

            std::map<juce::AudioChannelSet::ChannelType, float> result;
            mapper.onInputPositionMapped = [&] (std::uint16_t, const std::map<juce::AudioChannelSet::ChannelType, float>& dists)
            {
                result = dists;
            };

            mapper.mapInputPosition (1, { -1.0f, 0.0f }, 1.0f);

            expectEquals (result[juce::AudioChannelSet::right], 0.0f);
        }

        beginTest ("The channel argument is forwarded unchanged to the callback");
        {
            InputPositionMapper mapper;
            mapper.getAngleForChannelType = angleForTestChannel;
            mapper.setOutputIncludePositions ({ juce::AudioChannelSet::left });

            std::uint16_t gotChannel = 0;
            mapper.onInputPositionMapped = [&] (std::uint16_t channel, const std::map<juce::AudioChannelSet::ChannelType, float>&)
            {
                gotChannel = channel;
            };

            mapper.mapInputPosition (7, { 0.0f, 0.0f }, 0.0f);

            expectEquals ((int) gotChannel, 7);
        }
    }
};

static InputPositionMapperTest inputPositionMapperTest;

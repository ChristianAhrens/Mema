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
#include <JuceHeader.h>
#include <MemaClientCommon/TwoDFieldMultisliderComponent.h>

using namespace Mema;

//==============================================================================
// The 2D field must only report input-to-output states/values to its listeners (which forward them to Mema) when the user changes
// something -- not when levels/states are applied from the network, or when an input is (de)selected. The directionless (LFE) slider
// is where this used to go wrong: it was reconfigured with an async, notifying setValue().
class TwoDFieldMultisliderNotificationTest : public juce::UnitTest
{
public:
    TwoDFieldMultisliderNotificationTest() : juce::UnitTest ("TwoDFieldMultisliderComponent: no sends without user action", "Mema") {}

    void runTest() override
    {
        // 5.1: channel 4 is the LFE (the directionless slider)
        std::map<std::uint16_t, std::map<std::uint16_t, bool>> states;
        std::map<std::uint16_t, std::map<std::uint16_t, float>> levels;
        for (std::uint16_t out = 1; out <= 6; out++)
        {
            states[1][out] = true;
            levels[1][out] = 0.7f;
        }

        TwoDFieldMultisliderComponent field;
        field.setBounds(0, 0, 600, 400);
        field.setChannelConfiguration(juce::AudioChannelSet::create5point1());
        field.setIOCount({ 1, 6 });
        field.setControlsSize(35);

        auto sentStates = 0;
        auto sentValues = 0;
        field.onInputToOutputStatesChanged = [&](const auto&) { sentStates++; };
        field.onInputToOutputValuesChanged = [&](const auto&) { sentValues++; };

        auto pump = [] { juce::MessageManager::getInstance()->runDispatchLoopUntil(150); };

        beginTest("Applying states and levels received from the network does not send anything back");
        {
            field.setInputToOutputStates(states);
            field.setInputToOutputLevels(levels);
            pump();
            expectEquals(sentStates, 0);
            expectEquals(sentValues, 0);
        }

        beginTest("Selecting an input does not send anything");
        {
            field.selectInput(1, true);
            pump();
            expectEquals(sentStates, 0);
            expectEquals(sentValues, 0);
        }

        beginTest("Deselecting the input (directionless sliders return to relative control) does not send anything");
        {
            field.selectInput(1, false);
            pump();
            expectEquals(sentStates, 0);
            expectEquals(sentValues, 0);
        }
    }
};

static TwoDFieldMultisliderNotificationTest twoDFieldMultisliderNotificationTest;

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
#include <MemaClientCommon/FaderbankControlComponent.h>
#include <CustomLookAndFeel.h>
#include <ToggleStateSlider.h>

using namespace Mema;

//==============================================================================
class FaderbankControlComponentTest : public juce::UnitTest
{
public:
    FaderbankControlComponentTest() : juce::UnitTest ("FaderbankControlComponent: mute states and accent colour", "Mema") {}

    template <typename T>
    static void collect (juce::Component& parent, std::vector<T*>& result)
    {
        for (auto* child : parent.getChildren())
        {
            if (auto* typed = dynamic_cast<T*> (child))
                result.push_back (typed);
            collect (*child, result);
        }
    }

    static std::vector<juce::TextButton*> getMuteButtons (juce::Component& c)
    {
        std::vector<juce::TextButton*> all, mutes;
        collect (c, all);
        for (auto* b : all)
            if (b->getButtonText() == "M")
                mutes.push_back (b);
        return mutes;
    }

    void runTest() override
    {
        beginTest ("A mute click keeps the other mute states (the local map is not replaced by the single changed entry)");
        {
            FaderbankControlComponent c;
            c.setBounds (0, 0, 800, 600);
            c.setIOCount ({ 3, 2 });

            std::map<std::uint16_t, bool> lastReportedInputMutes;
            c.onInputMutesChanged = [&] (const std::map<std::uint16_t, bool>& m) { lastReportedInputMutes = m; };

            auto muteButtons = getMuteButtons (c); // 3 input mutes, then 2 output mutes
            expectEquals ((int) muteButtons.size(), 5);
            if (muteButtons.size() == 5)
            {
                muteButtons[0]->triggerClick(); // input 1 muted (triggerClick is handled asynchronously)
                juce::MessageManager::getInstance()->runDispatchLoopUntil (50);
                muteButtons[1]->triggerClick(); // input 2 muted
                juce::MessageManager::getInstance()->runDispatchLoopUntil (50);

                // only the changed entry is reported onwards ...
                expectEquals ((int) lastReportedInputMutes.size(), 1);
                // ... but the local state keeps both
                expect (c.getInputMuteStates().count (1) != 0 && c.getInputMuteStates().at (1));
                expect (c.getInputMuteStates().count (2) != 0 && c.getInputMuteStates().at (2));

                // a rebuild of all controls (e.g. by a size change) restores the buttons from the local state
                c.setControlsSize (MemaClientControlComponentBase::ControlsSize::M);
                muteButtons = getMuteButtons (c);
                expectEquals ((int) muteButtons.size(), 5);
                if (muteButtons.size() == 5)
                {
                    expect (muteButtons[0]->getToggleState(), "input 1 should still be muted after rebuild");
                    expect (muteButtons[1]->getToggleState(), "input 2 should still be muted after rebuild");
                    expect (! muteButtons[2]->getToggleState());
                }
            }
        }

        beginTest ("The accent colour of the matrix rotaries follows lookAndFeelChanged");
        {
            FaderbankControlComponent c;
            c.setBounds (0, 0, 800, 600);

            JUCEAppBasics::CustomLookAndFeel laf (JUCEAppBasics::CustomLookAndFeel::PS_Dark);
            laf.setColour (JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId, juce::Colours::green);
            c.setLookAndFeel (&laf);
            c.setIOCount ({ 2, 2 });

            std::vector<JUCEAppBasics::ToggleStateSlider*> sliders;
            collect (c, sliders);
            expectEquals ((int) sliders.size(), 4);

            laf.setColour (JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId, juce::Colours::red);
            c.lookAndFeelChanged();

            for (auto* s : sliders)
                expect (s->findColour (juce::Slider::ColourIds::trackColourId) == juce::Colours::red, "rotary accent was not updated");

            c.setLookAndFeel (nullptr);
        }
    }
};

static FaderbankControlComponentTest faderbankControlComponentTest;

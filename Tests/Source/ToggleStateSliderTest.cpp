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
#include <ToggleStateSlider.h>

//==============================================================================
// A click on a ToggleStateSlider (<= 1 px movement) only flips the toggle state. It must not also move the value to the
// mouse position, which the plain juce::Slider does on mouseDown. A real drag still has to change the value.
class ToggleStateSliderClickTest : public juce::UnitTest
{
public:
    ToggleStateSliderClickTest() : juce::UnitTest ("ToggleStateSlider: click toggles without changing the value", "Mema") {}

    static juce::MouseEvent makeEvent (juce::Component& c, juce::Point<float> pos, juce::Point<float> downPos, bool dragged)
    {
        auto source = juce::Desktop::getInstance().getMainMouseSource();
        auto now = juce::Time::getCurrentTime();
        return juce::MouseEvent (source, pos, juce::ModifierKeys::leftButtonModifier, juce::MouseInputSource::defaultPressure,
                                 juce::MouseInputSource::defaultOrientation, juce::MouseInputSource::defaultRotation,
                                 juce::MouseInputSource::defaultTiltX, juce::MouseInputSource::defaultTiltY,
                                 &c, &c, now, downPos, now, 1, dragged);
    }

    void checkClickAndDrag (juce::Slider::SliderStyle style, const juce::String& name)
    {
        JUCEAppBasics::ToggleStateSlider slider (style, juce::Slider::NoTextBox);
        slider.setBounds (0, 0, 100, 100);
        slider.setRange (0.0, 1.0, 0.01);
        slider.setValue (0.2, juce::dontSendNotification);
        slider.setToggleState (false, juce::dontSendNotification);

        auto toggleChanges = 0;
        slider.onToggleStateChange = [&] { toggleChanges++; };

        beginTest (name + ": a click on a distant position flips the toggle state and keeps the value");
        {
            auto clickPos = juce::Point<float> (90.0f, 10.0f);
            slider.mouseDown (makeEvent (slider, clickPos, clickPos, false));
            slider.mouseUp (makeEvent (slider, clickPos, clickPos, false));
            expect (slider.getToggleState());
            expectEquals (toggleChanges, 1);
            expectWithinAbsoluteError (slider.getValue(), 0.2, 0.0001);
        }

        beginTest (name + ": a drag still changes the value and does not flip the toggle state");
        {
            auto downPos = juce::Point<float> (50.0f, 50.0f);
            slider.mouseDown (makeEvent (slider, downPos, downPos, false));
            slider.mouseDrag (makeEvent (slider, { 50.0f, 10.0f }, downPos, true));
            slider.mouseDrag (makeEvent (slider, { 50.0f, 5.0f }, downPos, true));
            slider.mouseUp (makeEvent (slider, { 50.0f, 5.0f }, downPos, true));
            expect (std::abs (slider.getValue() - 0.2) > 0.01, "drag should have changed the value, is " + juce::String (slider.getValue()));
            expectEquals (toggleChanges, 1);
        }
    }

    void runTest() override
    {
        checkClickAndDrag (juce::Slider::LinearVertical, "Linear");
        checkClickAndDrag (juce::Slider::Rotary, "Rotary");
    }
};

static ToggleStateSliderClickTest toggleStateSliderClickTest;

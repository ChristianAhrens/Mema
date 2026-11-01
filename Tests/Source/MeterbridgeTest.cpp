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
#include <MemaProcessorEditor/MeterbridgeComponent.h>
#include <CustomLookAndFeel.h>

using namespace Mema;

//==============================================================================
// Both meterbridge directions draw the range text ("-80 ... 0 dBFS"). In the vertical direction it used to be rotated by 90 *radians*
// (juce::AffineTransform::rotated expects radians) and ended up outside of the component, i.e. was never visible.
class MeterbridgeRangeTextTest : public juce::UnitTest
{
public:
    MeterbridgeRangeTextTest() : juce::UnitTest ("MeterbridgeComponent: range text is drawn inside the component", "Mema") {}

    // number of pixels in the right part of the component that differ from the background
    static int countNonBackgroundPixelsRightPart (MeterbridgeComponent& c, juce::Colour background)
    {
        auto image = c.createComponentSnapshot (c.getLocalBounds(), false, 1.0f);
        auto count = 0;
        for (auto y = 0; y < image.getHeight(); ++y)
            for (auto x = (6 * image.getWidth()) / 10; x < image.getWidth(); ++x)
                if (image.getPixelAt (x, y) != background)
                    ++count;
        return count;
    }

    void runTest() override
    {
        JUCEAppBasics::CustomLookAndFeel laf (JUCEAppBasics::CustomLookAndFeel::PS_Dark);
        auto background = laf.findColour (juce::ResizableWindow::backgroundColourId);

        beginTest ("Vertical meterbridge shows the range text in its right part (channel labels and level bars are on the left, there is no signal)");
        {
            MeterbridgeComponent c (MeterbridgeComponent::Direction::Vertical);
            c.setLookAndFeel (&laf);
            c.setChannelCount (8);
            c.setBounds (0, 0, 120, 300);
            expect (countNonBackgroundPixelsRightPart (c, background) > 50, "range text not found inside the component");
            c.setLookAndFeel (nullptr);
        }
    }
};

static MeterbridgeRangeTextTest meterbridgeRangeTextTest;

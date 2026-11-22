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
#include <MemaClientCommon/TwoDFieldOutputComponent.h>
#include <CustomLookAndFeel.h>

using namespace Mema;

//==============================================================================
// Mema.Mo keeps one TwoDFieldOutputComponent and only changes its channel configuration when the user selects another format
// (MemaMoComponent::setOutputFieldVisuActive). The parent then sets unchanged bounds, which does not trigger resized() -- the areas
// and the speaker positions of the new format must be calculated nevertheless, i.e. look exactly like in a freshly created component.
class TwoDFieldOutputFormatChangeTest : public juce::UnitTest
{
public:
    TwoDFieldOutputFormatChangeTest() : juce::UnitTest ("TwoDFieldOutputComponent: format change recalculates the geometry", "Mema") {}

    static juce::Image snapshot (TwoDFieldOutputComponent& c) { return c.createComponentSnapshot (c.getLocalBounds(), false, 1.0f); }

    static int countDifferingPixels (const juce::Image& a, const juce::Image& b)
    {
        auto count = 0;
        for (auto y = 0; y < a.getHeight(); ++y)
            for (auto x = 0; x < a.getWidth(); ++x)
                if (a.getPixelAt (x, y) != b.getPixelAt (x, y))
                    ++count;
        return count;
    }

    void checkChange (const juce::AudioChannelSet& from, const juce::AudioChannelSet& to, const juce::String& description)
    {
        JUCEAppBasics::CustomLookAndFeel laf (JUCEAppBasics::CustomLookAndFeel::PS_Dark);
        juce::Rectangle<int> bounds (0, 0, 600, 400);

        TwoDFieldOutputComponent fresh;
        fresh.setLookAndFeel (&laf);
        fresh.setBounds (bounds);
        fresh.setChannelConfiguration (to);
        fresh.resized(); // what a new component gets from its parent

        TwoDFieldOutputComponent changed;
        changed.setLookAndFeel (&laf);
        changed.setBounds (bounds);
        changed.setChannelConfiguration (from);
        changed.resized();
        // the format change as done by MemaMoComponent: new configuration, then the parent sets the same bounds again (no resized() call)
        changed.setChannelConfiguration (to);
        changed.setBounds (bounds);

        beginTest (description);
        expectEquals (countDifferingPixels (snapshot (fresh), snapshot (changed)), 0);

        fresh.setLookAndFeel (nullptr);
        changed.setLookAndFeel (nullptr);
    }

    void runTest() override
    {
        checkChange (juce::AudioChannelSet::create5point0(), juce::AudioChannelSet::create7point0(), "5.0 -> 7.0 (same aspect ratio)");
        checkChange (juce::AudioChannelSet::create5point1point2(), juce::AudioChannelSet::create7point1point4(), "5.1.2 -> 7.1.4 (same aspect ratio)");
        checkChange (juce::AudioChannelSet::create5point1(), juce::AudioChannelSet::create7point1(), "5.1 -> 7.1 (same aspect ratio)");
    }
};

static TwoDFieldOutputFormatChangeTest twoDFieldOutputFormatChangeTest;

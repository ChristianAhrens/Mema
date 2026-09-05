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

// Placeholder test to prove the Mema test pipeline (Projucer export, build,
// execution, CI gating) end-to-end. Replace/extend with real coverage of
// Mema's logic as it gets extracted into testable, JUCE-UI-independent units.

#include <JuceHeader.h>

class SanityTest : public juce::UnitTest
{
public:
    SanityTest() : juce::UnitTest ("Sanity", "Mema") {}

    void runTest() override
    {
        beginTest ("Trivial arithmetic sanity check");
        expectEquals (1 + 1, 2);
    }
};

static SanityTest sanityTest;

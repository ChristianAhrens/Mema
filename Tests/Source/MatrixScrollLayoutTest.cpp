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
#include <MemaProcessorEditor/MemaProcessorEditor.h>

using namespace Mema;

//==============================================================================
// The editor must not grow beyond the screen with many channels: the crosspoint matrix scrolls, the layout decides when and how large.
class MatrixScrollLayoutTest : public juce::UnitTest
{
public:
    MatrixScrollLayoutTest() : juce::UnitTest ("MatrixScrollLayout::calculate()", "Mema") {}

    void runTest() override
    {
        const auto sb = 8;

        beginTest ("A matrix that fits is not scrolled and fills the available area");
        {
            auto layout = MatrixScrollLayout::calculate ({ 0, 0, 200, 100 }, { 0, 0, 400, 300 }, sb);
            expectEquals (layout.visibleWidth, 400);
            expectEquals (layout.visibleHeight, 300);
            expectEquals (layout.contentWidth, 400);
            expectEquals (layout.contentHeight, 300);
            expect (! layout.scrollsHorizontally() && ! layout.scrollsVertically());
        }

        beginTest ("A matrix that is too large for both directions scrolls in both and keeps its required size");
        {
            auto layout = MatrixScrollLayout::calculate ({ 0, 0, 1600, 1600 }, { 0, 0, 400, 300 }, sb);
            expectEquals (layout.visibleWidth, 400 - sb);
            expectEquals (layout.visibleHeight, 300 - sb);
            expectEquals (layout.contentWidth, 1600);
            expectEquals (layout.contentHeight, 1600);
            expect (layout.scrollsHorizontally() && layout.scrollsVertically());
        }

        beginTest ("Only one direction too large: only that direction scrolls");
        {
            auto layout = MatrixScrollLayout::calculate ({ 0, 0, 200, 1600 }, { 0, 0, 400, 300 }, sb);
            expectEquals (layout.visibleWidth, 400 - sb);
            expectEquals (layout.visibleHeight, 300);
            expectEquals (layout.contentWidth, 400 - sb);
            expect (! layout.scrollsHorizontally() && layout.scrollsVertically());
        }

        beginTest ("A vertical scrollbar that takes the width the matrix needs makes horizontal scrolling necessary (and vice versa)");
        {
            auto layout = MatrixScrollLayout::calculate ({ 0, 0, 395, 1600 }, { 0, 0, 400, 300 }, sb);
            expect (layout.scrollsHorizontally() && layout.scrollsVertically());
            expectEquals (layout.contentWidth, 395);

            layout = MatrixScrollLayout::calculate ({ 0, 0, 1600, 295 }, { 0, 0, 400, 300 }, sb);
            expect (layout.scrollsHorizontally() && layout.scrollsVertically());
            expectEquals (layout.contentHeight, 295);
        }

        beginTest ("An empty matrix needs nothing");
        {
            auto layout = MatrixScrollLayout::calculate ({}, { 0, 0, 400, 300 }, sb);
            expect (! layout.scrollsHorizontally() && ! layout.scrollsVertically());
            expectEquals (layout.contentWidth, 400);
            expectEquals (layout.contentHeight, 300);
        }
    }
};

static MatrixScrollLayoutTest matrixScrollLayoutTest;

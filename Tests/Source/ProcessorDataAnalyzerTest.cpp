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

// Coverage for Mema::ProcessorDataAnalyzer under the exact configuration Mema's
// own MemaProcessor uses for its on-device input/output level meters --
// setUseProcessingTypes(true, false, false), i.e. level processing with buffer
// processing turned OFF. (Mema.Mo's usage of the same shared class always pairs
// level with buffer processing; that path is covered separately in
// MemaMo/Tests/Source/ProcessorDataAnalyzerTest.cpp and is not affected by the
// bug below.)
//
// REGRESSION TEST FOR A KNOWN, NOT-YET-FIXED BUG: analyzeData()'s level (and
// spectrum) computation reads its input from m_centiSecondBuffer, which is only
// ever populated with the real signal inside the `isBufferProcessingUsed()`
// branch. With buffer processing disabled -- exactly MemaProcessor's
// configuration for m_inputDataAnalyzer/m_outputDataAnalyzer -- level metering
// reads whatever m_centiSecondBuffer already happened to contain (stale or
// uninitialised memory), not the signal just passed in. MemaProcessorEditor.cpp
// wires these analyzers directly to InputControlComponent/OutputControlComponent
// (Mema's own on-device level meters) via addInputListener()/addOutputListener(),
// so this means Mema's own on-device level meters currently do not show real
// levels.
//
// This test is expected to fail until that bug is fixed in
// ProcessorDataAnalyzer::analyzeData(). Per this project's convention, that fix
// belongs in its own separate commit, not folded into the commit that adds this
// test -- this test is meant to be red in the meantime, not weakened to pass.

#include <JuceHeader.h>
#include <MemaProcessor/ProcessorDataAnalyzer.h>

using namespace Mema;

class ProcessorDataAnalyzerMemaUsageTest : public juce::UnitTest
{
public:
    ProcessorDataAnalyzerMemaUsageTest() : juce::UnitTest ("ProcessorDataAnalyzer with Mema's own level-only configuration [bug]", "Mema") {}

    void runTest() override
    {
        beginTest ("A constant-amplitude buffer produces a matching level even with buffer processing disabled");

        ProcessorDataAnalyzer analyzer;
        analyzer.initializeParameters (48000.0, 480);
        // Mirrors MemaProcessor's constructor exactly:
        //   m_inputDataAnalyzer->setUseProcessingTypes(true, false, false);
        analyzer.setUseProcessingTypes (true, false, false);

        // exactly one centisecond's worth of samples at 48kHz (48000 * 0.01)
        juce::AudioBuffer<float> buffer (1, 480);
        for (int s = 0; s < 480; ++s)
            buffer.setSample (0, s, 0.5f);

        analyzer.analyzeData (buffer);

        auto level = analyzer.GetLevel().GetLevel (1);
        expectWithinAbsoluteError (level.peak, 0.5f, 1.0e-5f);
        expectWithinAbsoluteError (level.rms, 0.5f, 1.0e-5f);
    }
};

static ProcessorDataAnalyzerMemaUsageTest processorDataAnalyzerMemaUsageTest;

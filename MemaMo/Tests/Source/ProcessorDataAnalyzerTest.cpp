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

// Coverage for Mema::ProcessorDataAnalyzer -- the class Mema.Mo uses to turn the
// raw audio buffers streamed from Mema into level and spectrum data for its
// meterbridge/2D-field/waveform/spectrum visualisations. Level computation
// happens synchronously inside analyzeData() (verified below with an exact,
// hand-computed expected value), so no audio device or running message loop is
// needed. Only the level path is verified with exact numbers; the spectrum (FFT)
// path is exercised as a smoke test (it runs without crashing/NaN-ing and
// produces normalised values) rather than asserting exact per-bin numbers, since
// hand-deriving the expected output of the log-frequency-banded FFT would be
// fragile and error-prone to get right independently of the implementation.
//
// NOTE: constructing ProcessorDataAnalyzer starts a juce::Timer (setHoldTime() in
// its constructor), which lazily spins up JUCE's shared background TimerThread.
// This is standard, supported JUCE usage in a plain console app (Timer::~Timer()
// stops the timer automatically), but it is the first place in this test suite
// a Timer-derived object is constructed -- flagging in case it ever behaves
// differently than juce::JUCEApplication's already-proven-safe dummy-instance
// pattern used in *AppConfigurationTest.cpp.
//
// NOTE: level channels are 1-based (SetLevel(i + 1, ...) in analyzeData(), matching
// MeterbridgeComponent's `for (i = 1; i <= channelCount; ++i)`), while spectrum
// channels are 0-based (SetSpectrum(i, ...), matching SpectrumAudioComponent's
// `for (i = 0; i < numVisibleChannels; ++i)`). Both producer and consumer agree
// within each data type, so this isn't a functional bug, just an inconsistency
// between the two conventions worth knowing about if ever correlating a "channel
// N" between a level display and a spectrum display for the same physical input.

#include <JuceHeader.h>
#include <MemaProcessor/ProcessorDataAnalyzer.h>

using namespace Mema;

class ProcessorDataAnalyzerLevelTest : public juce::UnitTest
{
public:
    ProcessorDataAnalyzerLevelTest() : juce::UnitTest ("ProcessorDataAnalyzer level processing", "Mema.Mo") {}

    void runTest() override
    {
        beginTest ("analyzeData() is a no-op before initializeParameters()");
        {
            ProcessorDataAnalyzer analyzer;
            expect (! analyzer.IsInitialized());

            juce::AudioBuffer<float> buffer (1, 10);
            buffer.clear();
            analyzer.analyzeData (buffer); // should not crash, should not create any channel data
            expectEquals ((int) analyzer.GetLevel().GetChannelCount(), 0);
        }

        beginTest ("A constant-amplitude buffer produces the exact expected peak/RMS level");
        {
            ProcessorDataAnalyzer analyzer;
            analyzer.initializeParameters (48000.0, 480);
            analyzer.setUseProcessingTypes (true, false, false); // level only
            expect (analyzer.IsInitialized());

            // exactly one centisecond's worth of samples at 48kHz (48000 * 0.01)
            juce::AudioBuffer<float> buffer (1, 480);
            for (int s = 0; s < 480; ++s)
                buffer.setSample (0, s, 0.5f);

            analyzer.analyzeData (buffer);

            // level channels are 1-based -- see file header note
            auto level = analyzer.GetLevel().GetLevel (1);
            expectWithinAbsoluteError (level.peak, 0.5f, 1.0e-5f);
            expectWithinAbsoluteError (level.rms, 0.5f, 1.0e-5f); // RMS of a constant signal equals its magnitude
            expectWithinAbsoluteError (level.hold, 0.5f, 1.0e-5f); // hold = max(peak, previous hold=0)
        }

        beginTest ("Hold value stays at the previous peak when a quieter buffer follows (until FlushHold() fires)");
        {
            ProcessorDataAnalyzer analyzer;
            analyzer.initializeParameters (48000.0, 480);
            analyzer.setUseProcessingTypes (true, false, false);

            juce::AudioBuffer<float> loud (1, 480);
            for (int s = 0; s < 480; ++s)
                loud.setSample (0, s, 0.5f);
            analyzer.analyzeData (loud);

            juce::AudioBuffer<float> silence (1, 480);
            silence.clear();
            analyzer.analyzeData (silence);

            auto level = analyzer.GetLevel().GetLevel (1);
            expectWithinAbsoluteError (level.peak, 0.0f, 1.0e-6f);
            expectWithinAbsoluteError (level.hold, 0.5f, 1.0e-5f);
        }

        beginTest ("getGlobalMindB()/getGlobalMaxdB() match the documented -80..0 dB display range");
        {
            expectEquals (ProcessorDataAnalyzer::getGlobalMindB(), -80);
            expectEquals (ProcessorDataAnalyzer::getGlobalMaxdB(), 0);
        }
    }
};

static ProcessorDataAnalyzerLevelTest processorDataAnalyzerLevelTest;

//==============================================================================
class ProcessorDataAnalyzerSpectrumSmokeTest : public juce::UnitTest
{
public:
    ProcessorDataAnalyzerSpectrumSmokeTest() : juce::UnitTest ("ProcessorDataAnalyzer spectrum processing (smoke test)", "Mema.Mo") {}

    void runTest() override
    {
        beginTest ("Feeding enough samples to trigger at least one FFT produces finite, normalised spectrum values");

        ProcessorDataAnalyzer analyzer;
        analyzer.initializeParameters (48000.0, 480);
        analyzer.setUseProcessingTypes (false, false, true); // spectrum only

        // fftOrder is 12 (fftSize 4096) internally; feed more than that many silent
        // samples in one go so analyzeData()'s internal centisecond loop pushes at
        // least one full FFT window through processSpectrumForChannel().
        juce::AudioBuffer<float> buffer (1, 4096 + 480);
        buffer.clear();

        analyzer.analyzeData (buffer);

        // spectrum channels are 0-based -- see file header note
        const auto& spectrum = analyzer.GetSpectrum().GetSpectrum (0);

        expectEquals (spectrum.minFreq, 20.0f);
        expect (spectrum.maxFreq > spectrum.minFreq);
        expect (spectrum.freqRes > 0.0f);

        for (int i = 0; i < ProcessorSpectrumData::SpectrumBands::count; ++i)
        {
            expect (! std::isnan (spectrum.bandsPeak[i]) && ! std::isinf (spectrum.bandsPeak[i]), "spectrum band values must be finite");
            expect (spectrum.bandsPeak[i] >= 0.0f && spectrum.bandsPeak[i] <= 1.0f, "spectrum band values should be normalised to [0, 1]");
        }
    }
};

static ProcessorDataAnalyzerSpectrumSmokeTest processorDataAnalyzerSpectrumSmokeTest;

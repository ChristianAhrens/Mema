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

// Coverage for the pure data-holder classes exchanged between MemaProcessor and
// its analyzers/visualisers: ProcessorLevelData, ProcessorSpectrumData, and
// ProcessorAudioSignalData. None of these depend on a live audio device, network
// connection, or UI -- they are plain in-memory maps/buffers wrapping metering
// and signal data.

#include <JuceHeader.h>
#include <MemaProcessor/ProcessorLevelData.h>
#include <MemaProcessor/ProcessorSpectrumData.h>
#include <MemaProcessor/ProcessorAudioSignalData.h>

using namespace Mema;

class ProcessorLevelDataTest : public juce::UnitTest
{
public:
    ProcessorLevelDataTest() : juce::UnitTest ("ProcessorLevelData", "Mema") {}

    void runTest() override
    {
        beginTest ("Data type is set correctly on construction");
        {
            ProcessorLevelData data;
            expect (data.GetDataType() == AbstractProcessorData::Level);
        }

        beginTest ("GetLevel on an unset channel returns a silent default, not garbage");
        {
            ProcessorLevelData data;
            auto level = data.GetLevel (1);
            expectEquals (level.peak, 0.0f);
            expectEquals (level.rms, 0.0f);
            expectEquals (level.hold, 0.0f);
        }

        beginTest ("SetLevel/GetLevel round-trips a channel's values");
        {
            ProcessorLevelData data;
            ProcessorLevelData::LevelVal val (0.5f, 0.25f, 0.75f);
            data.SetLevel (3, val);

            auto got = data.GetLevel (3);
            expectEquals (got.peak, 0.5f);
            expectEquals (got.rms, 0.25f);
            expectEquals (got.hold, 0.75f);
            // dB conversion happens in the LevelVal(peak, rms, hold, infDb) constructor
            expectEquals (got.peakdB, juce::Decibels::gainToDecibels (0.5f, -100.0f));
        }

        beginTest ("SetChannelCount fills missing channels with silent defaults without touching existing ones");
        {
            ProcessorLevelData data;
            data.SetLevel (2, ProcessorLevelData::LevelVal (1.0f, 1.0f, 1.0f));

            data.SetChannelCount (4);

            expectEquals ((int) data.GetChannelCount(), 4);
            // the channel that was already set keeps its value...
            expectEquals (data.GetLevel (2).peak, 1.0f);
            // ...while the newly-added ones are silent
            expectEquals (data.GetLevel (1).peak, 0.0f);
            expectEquals (data.GetLevel (3).peak, 0.0f);
            expectEquals (data.GetLevel (4).peak, 0.0f);
        }

        beginTest ("SetChannelCount with a smaller count does not shrink the map [documents current behaviour]");
        {
            // GetChannelCount() is just m_levelMap.size() and SetChannelCount() only ever adds
            // missing entries (1..count) -- it never removes entries above a smaller count. So
            // shrinking the channel count is a no-op as far as the stored data is concerned. Not
            // necessarily a bug (channel counts only ever grow for a given audio device in
            // practice), but worth having pinned down explicitly.
            ProcessorLevelData data;
            data.SetChannelCount (5);
            expectEquals ((int) data.GetChannelCount(), 5);

            data.SetChannelCount (2);
            expectEquals ((int) data.GetChannelCount(), 5);
        }

        beginTest ("LevelVal factor helpers scale linearly between minusInfdb and 0dB");
        {
            auto infDb = -100.0f;
            ProcessorLevelData::LevelVal silence (0.0f, 0.0f, 0.0f, infDb);
            ProcessorLevelData::LevelVal fullScale (1.0f, 1.0f, 1.0f, infDb);

            expectWithinAbsoluteError (silence.GetFactorPEAKdB(), 0.0f, 1.0e-6f);
            expectWithinAbsoluteError (fullScale.GetFactorPEAKdB(), 1.0f, 1.0e-6f);
        }
    }
};

static ProcessorLevelDataTest processorLevelDataTest;

//==============================================================================
class ProcessorSpectrumDataTest : public juce::UnitTest
{
public:
    ProcessorSpectrumDataTest() : juce::UnitTest ("ProcessorSpectrumData", "Mema") {}

    void runTest() override
    {
        beginTest ("Data type is set correctly on construction");
        {
            ProcessorSpectrumData data;
            expect (data.GetDataType() == AbstractProcessorData::Spectrum);
        }

        beginTest ("GetSpectrum on an unset channel lazily inserts and returns a sane default");
        {
            ProcessorSpectrumData data;
            expectEquals ((int) data.GetChannelCount(), 0);

            const auto& spectrum = data.GetSpectrum (1);

            expectEquals (spectrum.minFreq, 20.0f);
            expectEquals (spectrum.maxFreq, 20000.0f);
            expectEquals ((int) ProcessorSpectrumData::SpectrumBands::count, 512);
            expectEquals (spectrum.bandsPeak[0], 0.0f);
            expectEquals (spectrum.bandsPeak[511], 0.0f);

            // the lazy insertion is a real side effect: the channel now exists
            expectEquals ((int) data.GetChannelCount(), 1);
        }

        beginTest ("SetSpectrum/GetSpectrum round-trips per-channel data");
        {
            ProcessorSpectrumData data;
            ProcessorSpectrumData::SpectrumBands bands;
            bands.bandsPeak[10] = 0.42f;
            bands.bandsHold[10] = 0.5f;
            bands.mindB = -80.0f;
            bands.maxdB = 0.0f;

            data.SetSpectrum (2, bands);

            const auto& got = data.GetSpectrum (2);
            expectEquals (got.bandsPeak[10], 0.42f);
            expectEquals (got.bandsHold[10], 0.5f);
            expectEquals (got.mindB, -80.0f);
        }

        beginTest ("SetChannelCount fills missing channels without touching existing ones");
        {
            ProcessorSpectrumData data;
            ProcessorSpectrumData::SpectrumBands bands;
            bands.bandsPeak[0] = 1.0f;
            data.SetSpectrum (1, bands);

            data.SetChannelCount (3);

            expectEquals ((int) data.GetChannelCount(), 3);
            expectEquals (data.GetSpectrum (1).bandsPeak[0], 1.0f);
            expectEquals (data.GetSpectrum (2).bandsPeak[0], 0.0f);
        }
    }
};

static ProcessorSpectrumDataTest processorSpectrumDataTest;

//==============================================================================
class ProcessorAudioSignalDataTest : public juce::UnitTest
{
public:
    ProcessorAudioSignalDataTest() : juce::UnitTest ("ProcessorAudioSignalData", "Mema") {}

    void runTest() override
    {
        beginTest ("Data type is set correctly on construction");
        {
            ProcessorAudioSignalData data;
            expect (data.GetDataType() == AbstractProcessorData::AudioSignal);
            expectEquals ((int) data.GetSampleRate(), 0);
        }

        beginTest ("SetSampleRate/GetSampleRate round-trips");
        {
            ProcessorAudioSignalData data;
            data.SetSampleRate (48000);
            expectEquals ((int) data.GetSampleRate(), 48000);
        }

        beginTest ("SetChannelCount resizes the underlying buffer and preserves existing samples");
        {
            ProcessorAudioSignalData data;
            data.setSize (2, 16);
            data.setSample (0, 0, 0.5f);
            data.setSample (1, 0, -0.5f);

            data.SetChannelCount (4);

            expectEquals ((int) data.GetChannelCount(), 4);
            expectEquals (data.getNumSamples(), 16);
            // keepExistingContent=true in SetChannelCount() -- original samples must survive
            expectEquals (data.getSample (0, 0), 0.5f);
            expectEquals (data.getSample (1, 0), -0.5f);
        }
    }
};

static ProcessorAudioSignalDataTest processorAudioSignalDataTest;

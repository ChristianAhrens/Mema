/* Copyright (c) 2024-2025, Christian Ahrens
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

#include "ProcessorDataAnalyzer.h"

namespace Mema
{

#define USE_LEVEL_PROCESSING
#define USE_BUFFER_PROCESSING
//#define USE_SPECTRUM_PROCESSING


//==============================================================================
ProcessorDataAnalyzer::ProcessorDataAnalyzer() :
	m_fwdFFT(fftOrder),
	m_windowF(fftSize, dsp::WindowingFunction<float>::hann)
{
	m_FFTdataPos = 0;
	zeromem(m_FFTdata, sizeof(m_FFTdata));

	setHoldTime(500);
}

ProcessorDataAnalyzer::~ProcessorDataAnalyzer()
{

}

void ProcessorDataAnalyzer::initializeParameters(double sampleRate, int bufferSize)
{
	m_sampleRate = static_cast<unsigned long>(sampleRate);
	m_samplesPerCentiSecond = static_cast<int>(sampleRate * 0.01f);
	m_bufferSize = bufferSize;
	m_missingSamplesForCentiSecond = static_cast<int>(m_samplesPerCentiSecond + 0.5f);
	m_centiSecondBuffer.setSize(2, m_missingSamplesForCentiSecond, false, true, false);
}

void ProcessorDataAnalyzer::clearParameters()
{	
	m_sampleRate = 0;
	m_samplesPerCentiSecond = 0;
	m_bufferSize = 0;
	m_centiSecondBuffer.clear();
	m_missingSamplesForCentiSecond = 0;
}

void ProcessorDataAnalyzer::setHoldTime(int holdTimeMs)
{
	m_holdTimeMs = holdTimeMs;

	startTimer(m_holdTimeMs);
}

void ProcessorDataAnalyzer::addListener(Listener* listener)
{
	std::lock_guard<std::mutex> lock(m_callbackListenersMutex);
	m_callbackListeners.add(listener);
}

void ProcessorDataAnalyzer::removeListener(Listener* listener)
{
	std::lock_guard<std::mutex> lock(m_callbackListenersMutex);
	m_callbackListeners.remove(m_callbackListeners.indexOf(listener));
}

void ProcessorDataAnalyzer::analyzeData(const juce::AudioBuffer<float>& buffer)
{
	if (!IsInitialized())
		return;

	int numChannels = buffer.getNumChannels();

	if (numChannels != m_centiSecondBuffer.getNumChannels())
		m_centiSecondBuffer.setSize(numChannels, m_samplesPerCentiSecond, false, true, true);
	if (m_sampleRate != m_centiSecondBuffer.GetSampleRate())
		m_centiSecondBuffer.SetSampleRate(m_sampleRate);

	int availableSamples = buffer.getNumSamples();

	int readPos = 0;
	int writePos = m_samplesPerCentiSecond - m_missingSamplesForCentiSecond;
	while (availableSamples >= m_missingSamplesForCentiSecond)
	{
		for (int i = 0; i < numChannels; ++i)
		{
#ifdef USE_BUFFER_PROCESSING
			// generate signal buffer data
			m_centiSecondBuffer.copyFrom(i, writePos, buffer.getReadPointer(i) + readPos, m_missingSamplesForCentiSecond);
#endif

#ifdef USE_LEVEL_PROCESSING
			// generate level data
			auto peak = m_centiSecondBuffer.getMagnitude(i, 0, m_samplesPerCentiSecond);
			auto rms = m_centiSecondBuffer.getRMSLevel(i, 0, m_samplesPerCentiSecond);
			auto hold = std::max(peak, m_level.GetLevel(i + 1).hold);
			m_level.SetLevel(i + 1, ProcessorLevelData::LevelVal(peak, rms, hold, static_cast<float>(getGlobalMindB())));
#endif

#ifdef USE_SPECTRUM_PROCESSING
			// generate spectrum data
			{
				int unprocessedSamples = 0;
				if (m_FFTdataPos < fftSize)
				{
					int missingSamples = fftSize - m_FFTdataPos;
					if (missingSamples < m_samplesPerCentiSecond)
					{
						memcpy(m_FFTdata, m_centiSecondBuffer.getReadPointer(i), missingSamples);
						m_FFTdataPos += missingSamples;
						unprocessedSamples = m_samplesPerCentiSecond - missingSamples;
					}
					else
					{
						memcpy(m_FFTdata, m_centiSecondBuffer.getReadPointer(i), m_samplesPerCentiSecond);
						m_FFTdataPos += m_samplesPerCentiSecond;
					}
				}

				if (m_FFTdataPos >= fftSize)
				{
					m_windowF.multiplyWithWindowingTable(m_FFTdata, fftSize);
					m_fwdFFT.performFrequencyOnlyForwardTransform(m_FFTdata);
					ProcessorSpectrumData::SpectrumBands spectrumBands = m_spectrum.GetSpectrum(i);

					spectrumBands.mindB = static_cast<float>(getGlobalMindB());
					spectrumBands.maxdB = static_cast<float>(getGlobalMaxdB());

					spectrumBands.minFreq = static_cast<float>(m_sampleRate / ProcessorSpectrumData::SpectrumBands::count);
					spectrumBands.maxFreq = static_cast<float>(m_sampleRate / 2);
					spectrumBands.freqRes = static_cast<float>((spectrumBands.maxFreq - spectrumBands.minFreq) / ProcessorSpectrumData::SpectrumBands::count);

					auto spectrumStepWidth = static_cast<int>(0.5f * (fftSize / ProcessorSpectrumData::SpectrumBands::count));
					auto spectrumPos = 0;
					for (int j = 0; j < ProcessorSpectrumData::SpectrumBands::count && spectrumPos < fftSize; ++j)
					{
						float spectrumVal = 0;

						for (int k = 0; k < spectrumStepWidth; ++k, ++spectrumPos)
							spectrumVal += m_FFTdata[spectrumPos];
						spectrumVal = spectrumVal / spectrumStepWidth;

						auto leveldB = jlimit(spectrumBands.mindB, spectrumBands.maxdB, Decibels::gainToDecibels(spectrumVal));
						auto level = jmap(leveldB, spectrumBands.mindB, spectrumBands.maxdB, 0.0f, 1.0f);

						spectrumBands.bandsPeak[j] = level;
						spectrumBands.bandsHold[j] = std::max(level, spectrumBands.bandsHold[j]);
					}

					m_spectrum.SetSpectrum(i, spectrumBands);

					zeromem(m_FFTdata, sizeof(m_FFTdata));
					m_FFTdataPos = 0;
				}

				if (unprocessedSamples != 0)
				{
					memcpy(m_FFTdata, m_centiSecondBuffer.getReadPointer(i, m_samplesPerCentiSecond - unprocessedSamples), unprocessedSamples);
					m_FFTdataPos += unprocessedSamples;
				}
			}
#endif
		}

#ifdef USE_LEVEL_PROCESSING
		BroadcastData(&m_level);
#endif
#ifdef USE_BUFFER_PROCESSING
		BroadcastData(&m_centiSecondBuffer);
#endif
#ifdef USE_SPECTRUM_PROCESSING
		BroadcastData(&m_spectrum);
#endif

		readPos += m_missingSamplesForCentiSecond;
		availableSamples -= m_missingSamplesForCentiSecond;

		m_missingSamplesForCentiSecond = m_samplesPerCentiSecond;

		writePos = m_samplesPerCentiSecond - m_missingSamplesForCentiSecond;

		if (availableSamples <= 0)
			break;
	}

	if (availableSamples > 0)
	{
		for (int i = 0; i < numChannels; ++i)
		{
			m_centiSecondBuffer.copyFrom(i, writePos, buffer.getReadPointer(i) + readPos, availableSamples);
		}
		m_missingSamplesForCentiSecond -= availableSamples;
	}
}

void ProcessorDataAnalyzer::BroadcastData(AbstractProcessorData* data)
{
	std::lock_guard<std::mutex> lock(m_callbackListenersMutex);
	for (Listener* l : m_callbackListeners)
		l->processingDataChanged(data);
}

void ProcessorDataAnalyzer::timerCallback()
{
	FlushHold();
}

void ProcessorDataAnalyzer::FlushHold()
{
	// clear level hold values
	auto channelCount = static_cast<int>(m_level.GetChannelCount());
	for (auto i = 0; i < channelCount; ++i)
	{
		m_level.SetLevel(i + 1, ProcessorLevelData::LevelVal(0.0f, 0.0f, 0.0f, static_cast<float>(getGlobalMindB())));
	}

	// clear spectrum hold values	auto channelCount = m_level.GetChannelCount();
	channelCount = static_cast<int>(m_spectrum.GetChannelCount());
	for (auto i = 0; i < channelCount; ++i)
	{
		ProcessorSpectrumData::SpectrumBands spectrumBands = m_spectrum.GetSpectrum(i);
		for (auto j = 0; j < ProcessorSpectrumData::SpectrumBands::count; ++j)
		{
			spectrumBands.bandsPeak[j] = 0.0f;
			spectrumBands.bandsHold[j] = 0.0f;
		}

		m_spectrum.SetSpectrum(i, spectrumBands);
	}
}

} // namespace Mema
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
#define USE_SPECTRUM_PROCESSING


//==============================================================================
ProcessorDataAnalyzer::ProcessorDataAnalyzer() :
	m_fwdFFT(fftOrder),
	m_windowF(fftSize, dsp::WindowingFunction<float>::hann)
{
	setHoldTime(1000);
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

    // Ensure per-channel FFT buffers are sized correctly
#ifdef USE_SPECTRUM_PROCESSING
    if (m_FFTdata.size() != numChannels)
    {
        m_FFTdata.resize(numChannels);
        m_FFTdataPos.resize(numChannels, 0);
        for (auto& channelFFTdata : m_FFTdata)
            channelFFTdata.resize(fftSize * 2, 0.0f);
    }
#endif

    int availableSamples = buffer.getNumSamples();
    int readPos = 0;

    while (availableSamples >= m_missingSamplesForCentiSecond)
    {
        int writePos = m_samplesPerCentiSecond - m_missingSamplesForCentiSecond;

        for (int i = 0; i < numChannels; ++i)
        {
#ifdef USE_BUFFER_PROCESSING
            // Generate signal buffer data
            m_centiSecondBuffer.copyFrom(i, writePos, buffer.getReadPointer(i) + readPos, m_missingSamplesForCentiSecond);
#endif

#ifdef USE_LEVEL_PROCESSING
            // Generate level data
            auto peak = m_centiSecondBuffer.getMagnitude(i, 0, m_samplesPerCentiSecond);
            auto rms = m_centiSecondBuffer.getRMSLevel(i, 0, m_samplesPerCentiSecond);
            auto hold = std::max(peak, m_level.GetLevel(i + 1).hold);
            m_level.SetLevel(i + 1, ProcessorLevelData::LevelVal(peak, rms, hold, static_cast<float>(getGlobalMindB())));
#endif

#ifdef USE_SPECTRUM_PROCESSING
            // Generate spectrum data - all channels always process their audio data
            // The FFT buffer accumulates samples for all channels
            processSpectrumForChannel(i, m_centiSecondBuffer.getReadPointer(i), m_samplesPerCentiSecond);
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
    }

    // Handle remaining samples
    if (availableSamples > 0)
    {
        int writePos = m_samplesPerCentiSecond - m_missingSamplesForCentiSecond;
        for (int i = 0; i < numChannels; ++i)
        {
            m_centiSecondBuffer.copyFrom(i, writePos, buffer.getReadPointer(i) + readPos, availableSamples);
        }
        m_missingSamplesForCentiSecond -= availableSamples;
    }
}

void ProcessorDataAnalyzer::processSpectrumForChannel(int channelIndex, const float* channelData, int numSamples)
{
    int samplesProcessed = 0;

    // Fill FFT buffer until we have enough samples
    while (samplesProcessed < numSamples)
    {
        int samplesNeeded = fftSize - m_FFTdataPos[channelIndex];
        int samplesAvailable = numSamples - samplesProcessed;
        int samplesToCopy = std::min(samplesNeeded, samplesAvailable);

        // Copy samples into FFT buffer using JUCE's optimized copy
        juce::FloatVectorOperations::copy(
            m_FFTdata[channelIndex].data() + m_FFTdataPos[channelIndex],
            channelData + samplesProcessed,
            samplesToCopy
        );

        m_FFTdataPos[channelIndex] += samplesToCopy;
        samplesProcessed += samplesToCopy;

        // When we have enough samples, perform FFT
        if (m_FFTdataPos[channelIndex] >= fftSize)
        {
            performFFTAndUpdateSpectrum(channelIndex);

            // 25% overlap - good balance between smoothness and CPU usage
            const int hopSize = (fftSize * 3) / 4;

            // Shift the buffer efficiently
            juce::FloatVectorOperations::copy(
                m_FFTdata[channelIndex].data(),
                m_FFTdata[channelIndex].data() + hopSize,
                fftSize - hopSize
            );

            m_FFTdataPos[channelIndex] = fftSize - hopSize;
        }
    }
}

void ProcessorDataAnalyzer::performFFTAndUpdateSpectrum(int channelIndex)
{
    float* fftData = m_FFTdata[channelIndex].data();

    // Apply windowing function
    m_windowF.multiplyWithWindowingTable(fftData, fftSize);

    // Perform FFT
    m_fwdFFT.performFrequencyOnlyForwardTransform(fftData);

    // Get spectrum bands for this channel
    ProcessorSpectrumData::SpectrumBands spectrumBands = m_spectrum.GetSpectrum(channelIndex);
    spectrumBands.mindB = static_cast<float>(getGlobalMindB());
    spectrumBands.maxdB = static_cast<float>(getGlobalMaxdB());

    const float nyquistFreq = m_sampleRate * 0.5f;

    const float minDisplayFreq = 20.0f;
    const float maxDisplayFreq = std::min(20000.0f, nyquistFreq);

    spectrumBands.minFreq = minDisplayFreq;
    spectrumBands.maxFreq = maxDisplayFreq;
    spectrumBands.freqRes = (maxDisplayFreq - minDisplayFreq) / ProcessorSpectrumData::SpectrumBands::count;

    const int usableFFTBins = fftSize / 2;
    const float binFrequency = m_sampleRate / static_cast<float>(fftSize);

    const float fftScale = 1.0f / static_cast<float>(fftSize);
    const float windowCompensation = 2.0f;

    // With overlap still use smoothing for visual nicety
    const float smoothingFactor = 0.6f;

    // Pre-calculate log values for efficiency
    const float invBandCount = 1.0f / ProcessorSpectrumData::SpectrumBands::count;

    for (int bandIndex = 0; bandIndex < ProcessorSpectrumData::SpectrumBands::count; ++bandIndex)
    {
        // Optimized logarithmic frequency calculation
        float t = bandIndex * invBandCount;
        float bandStartFreq = minDisplayFreq * std::pow(maxDisplayFreq / minDisplayFreq, t);
        float bandEndFreq = minDisplayFreq * std::pow(maxDisplayFreq / minDisplayFreq, t + invBandCount);

        int startBin = static_cast<int>(bandStartFreq / binFrequency);
        int endBin = static_cast<int>(bandEndFreq / binFrequency);

        startBin = juce::jlimit(0, usableFFTBins - 1, startBin);
        endBin = juce::jlimit(startBin + 1, usableFFTBins, endBin);

        int numBins = endBin - startBin;

        // Calculate RMS
        float bandSumSquared = 0.0f;
        for (int bin = startBin; bin < endBin; ++bin)
        {
            float magnitude = fftData[bin] * fftScale * windowCompensation;
            bandSumSquared += magnitude * magnitude;
        }

        // Calculate RMS value
        float rmsValue = std::sqrt(bandSumSquared / numBins);

        // Convert to dB with proper floor to avoid log(0)
        const float minMagnitude = 0.00001f; // approximately -100 dB
        rmsValue = std::max(rmsValue, minMagnitude);

        float leveldB = juce::Decibels::gainToDecibels(rmsValue);
        leveldB = juce::jlimit(spectrumBands.mindB, spectrumBands.maxdB, leveldB);
        float normalizedLevel = juce::jmap(leveldB, spectrumBands.mindB, spectrumBands.maxdB, 0.0f, 1.0f);

        // Light smoothing with overlap
        float previousLevel = spectrumBands.bandsPeak[bandIndex];
        float smoothedLevel;

        if (normalizedLevel > previousLevel)
        {
            // Fast attack
            smoothedLevel = 0.1f * previousLevel + 0.9f * normalizedLevel;
        }
        else
        {
            // Moderate decay with overlap
            smoothedLevel = smoothingFactor * previousLevel + (1.0f - smoothingFactor) * normalizedLevel;
        }

        spectrumBands.bandsPeak[bandIndex] = smoothedLevel;
        spectrumBands.bandsHold[bandIndex] = std::max(smoothedLevel, spectrumBands.bandsHold[bandIndex]);
    }

    m_spectrum.SetSpectrum(channelIndex, spectrumBands);
    juce::FloatVectorOperations::clear(fftData, fftSize * 2);
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
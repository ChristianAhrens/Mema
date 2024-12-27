/* Copyright (c) 2024, Christian Ahrens
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

#pragma once

#include <JuceHeader.h>

#include "ProcessorAudioSignalData.h"
#include "ProcessorLevelData.h"
#include "ProcessorSpectrumData.h"


namespace Mema
{

//==============================================================================
/*
*/
class ProcessorDataAnalyzer :    public Timer
{
public:
    class Listener
    {
    public:
        virtual ~Listener() {};

        virtual void processingDataChanged(AbstractProcessorData* data) = 0;
    };

public:
    ProcessorDataAnalyzer();
    ~ProcessorDataAnalyzer();

    //==============================================================================
    void initializeParameters(double sampleRate, int bufferSize);
    void clearParameters();

    void setHoldTime(int holdTimeMs);

    ProcessorAudioSignalData& GetCentiSecondBuffer() { return m_centiSecondBuffer; };
    ProcessorLevelData& GetLevel() { return m_level; };
    ProcessorSpectrumData& GetSpectrum() { return m_spectrum; };
    String& GetName() { return m_Name; };

    bool IsInitialized() { return (m_bufferSize != 0 && m_sampleRate != 0); };

    //==============================================================================
    void addListener(Listener* listener);
    void removeListener(Listener* listener);

    //==============================================================================
    void analyzeData(const AudioBuffer<float>& buffer);

    //==============================================================================
    void timerCallback() override;

    //==============================================================================
    enum dBRange
    {
        min = -80,
        max = 0,
    };

    static int getGlobalMindB()
    {
        return dBRange::min;
    }

    static int getGlobalMaxdB()
    {
        return dBRange::max;
    }

private:
    void BroadcastData(AbstractProcessorData* data);
    void FlushHold();

    ProcessorAudioSignalData    m_centiSecondBuffer;
    ProcessorLevelData          m_level;
    ProcessorSpectrumData       m_spectrum;

    juce::String                m_Name;
    juce::Array<Listener*>      m_callbackListeners;
    std::mutex                  m_callbackListenersMutex;

    //==============================================================================
    CriticalSection     m_readLock;

    float**             m_processorChannels;

    unsigned long       m_sampleRate = 0;
    int                 m_samplesPerCentiSecond = 0;
    int                 m_bufferSize = 0;
    int                 m_missingSamplesForCentiSecond = 0;

    //==============================================================================
    enum
    {
        fftOrder = 12,
        fftSize = 1 << fftOrder
    };
    dsp::FFT                                    m_fwdFFT;
    dsp::WindowingFunction<float>               m_windowF;
    float                                       m_FFTdata[2 * fftSize];
    int                                         m_FFTdataPos;

    int                                         m_holdTimeMs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessorDataAnalyzer)
};

} // namespace Mema

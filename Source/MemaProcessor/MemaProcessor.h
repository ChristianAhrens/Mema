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

#include "ProcessorDataAnalyzer.h"
#include "../MemaEditor/MemaEditor.h"
#include "../AppConfiguration.h"


namespace Mema
{

class InterprocessConnectionImpl;
class InterprocessConnectionServerImpl;
class MemaChannelCommander;
class MemaInputCommander;
class MemaOutputCommander;
class MemaCrosspointCommander;
#if JUCE_WINDOWS
struct ServiceAdvertiser;
#endif


//==============================================================================
class ResizeableWindowWithTitleBarAndCloseCallback : public juce::ResizableWindow
{
public:
    ResizeableWindowWithTitleBarAndCloseCallback() : juce::ResizableWindow("", false) {};
    ResizeableWindowWithTitleBarAndCloseCallback(const String& name, bool addToDesktop) : juce::ResizableWindow(name, addToDesktop) {};
    ~ResizeableWindowWithTitleBarAndCloseCallback() { if (onClosed) onClosed(); };

    //==============================================================================
    int getDesktopWindowStyleFlags() const override
    {
        int styleFlags = juce::ComponentPeer::windowAppearsOnTaskbar
            | juce::ComponentPeer::windowHasDropShadow
            | juce::ComponentPeer::windowHasTitleBar
            | juce::ComponentPeer::windowHasCloseButton;

        return styleFlags;
    }

    void userTriedToCloseWindow() override
    {
        if (onClosed)
            onClosed();
    };

    //==============================================================================
    std::function<void()> onClosed;
};

//==============================================================================
class MemaProcessor :   public juce::AudioProcessor,
					    public juce::AudioIODeviceCallback,
                        public juce::MessageListener,
                        public juce::ChangeListener,
                        public AppConfiguration::XmlConfigurableElement
{
public:
    MemaProcessor(XmlElement* stateXml);
    ~MemaProcessor();

    //==============================================================================
    void addInputListener(ProcessorDataAnalyzer::Listener* listener);
    void removeInputListener(ProcessorDataAnalyzer::Listener* listener);
    void addOutputListener(ProcessorDataAnalyzer::Listener* listener);
    void removeOutputListener(ProcessorDataAnalyzer::Listener* listener);

    //==============================================================================
    void addInputCommander(MemaInputCommander* commander);
    void initializeInputCommander(MemaInputCommander* commander);
    void removeInputCommander(MemaInputCommander* commander);

    void addOutputCommander(MemaOutputCommander* commander);
    void initializeOutputCommander(MemaOutputCommander* commander);
    void removeOutputCommander(MemaOutputCommander* comander);

    void addCrosspointCommander(MemaCrosspointCommander* commander);
    void initializeCrosspointCommander(MemaCrosspointCommander* commander);
    void removeCrosspointCommander(MemaCrosspointCommander* comander);

    //==============================================================================
    bool getInputMuteState(int channelNumber);
    void setInputMuteState(int channelNumber, bool muted, MemaChannelCommander* sender = nullptr);
    
    bool getMatrixCrosspointEnabledValue(int inputNumber, int outputNumber);
    void setMatrixCrosspointEnabledValue(int inputNumber, int outputNumber, bool enabled, MemaChannelCommander* sender = nullptr);

    float getMatrixCrosspointFactorValue(int inputNumber, int outputNumber);
    void setMatrixCrosspointFactorValue(int inputNumber, int outputNumber, float factor, MemaChannelCommander* sender = nullptr);

    bool getOutputMuteState(int channelNumber);
    void setOutputMuteState(int channelNumber, bool muted, MemaChannelCommander* sender = nullptr);

    void setChannelCounts(int inputChannelCount, int outputChannelCount);

    //==============================================================================
    bool setPlugin(const juce::PluginDescription& pluginDescription);
    juce::PluginDescription getPluginDescription();
    void setPluginEnabledState(bool enabled);
    bool isPluginEnabled();
    void setPluginPrePostState(bool post);
    bool isPluginPost();
    void clearPlugin();
    void openPluginEditor();
    void closePluginEditor(bool deleteEditorWindow = true);
    std::function<void(const juce::PluginDescription&)> onPluginSet;

    //==============================================================================
    AudioDeviceManager* getDeviceManager();

    //==============================================================================
    std::map<int, std::pair<double, bool>> getNetworkHealth();

    //==============================================================================
    const String getName() const override;
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    void releaseResources() override;
    void processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) override;

    double getTailLengthSeconds() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const String getProgramName(int index) override;
    void changeProgramName(int index, const String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    //==============================================================================
    void audioDeviceIOCallbackWithContext (const float* const* inputChannelData,
                                           int numInputChannels,
                                           float* const* outputChannelData,
                                           int numOutputChannels,
                                           int numSamples,
                                           const AudioIODeviceCallbackContext& context) override;
    
    void audioDeviceAboutToStart(AudioIODevice* device) override;
    void audioDeviceStopped() override;

    //==============================================================================
    void changeListenerCallback(ChangeBroadcaster* source) override;

    //==============================================================================
    void handleMessage(const Message& message) override;

    //==============================================================================
    std::unique_ptr<XmlElement> createStateXml() override;
    bool setStateXml(XmlElement* stateXml) override;

    //==============================================================================
    void environmentChanged();

    //==============================================================================
    static constexpr int s_maxChannelCount = 64;
    static constexpr int s_maxNumSamples = 1024;

    static constexpr int s_minInputsCount = 1;
    static constexpr int s_minOutputsCount = 1;

protected:
    //==============================================================================
    void initializeCtrlValues(int inputCount, int outputCount);

private:
    //==============================================================================
    juce::String    m_Name;

    //==============================================================================
    juce::CriticalSection   m_audioDeviceIOCallbackLock;

    float** m_processorChannels;

    //==============================================================================
    std::unique_ptr<AudioDeviceManager> m_deviceManager;

    //==============================================================================
    std::unique_ptr<ProcessorDataAnalyzer>  m_inputDataAnalyzer;
    std::unique_ptr<ProcessorDataAnalyzer>  m_outputDataAnalyzer;

    //==============================================================================
    std::vector<MemaInputCommander*>    m_inputCommanders;
    std::vector<MemaOutputCommander*>   m_outputCommanders;
    std::vector<MemaCrosspointCommander*>   m_crosspointCommanders;

    //==============================================================================
    std::map<int, bool> m_inputMuteStates;
    std::map<int, bool> m_outputMuteStates;

    //==============================================================================
    int m_inputChannelCount{ 1 };
    int m_outputChannelCount{ 1 };

    //==============================================================================
    std::map<int, std::map<int, std::pair<bool, float>>>  m_matrixCrosspointValues;

    //==============================================================================
    std::unique_ptr<MemaEditor>  m_processorEditor;

    //==============================================================================
    juce::CriticalSection                                           m_pluginProcessingLock;
    std::unique_ptr<juce::AudioPluginInstance>                      m_pluginInstance;
    bool                                                            m_pluginEnabled = false;
    bool                                                            m_pluginPost = false;
    std::unique_ptr<ResizeableWindowWithTitleBarAndCloseCallback>   m_pluginEditorWindow;

    //==============================================================================
#if JUCE_WINDOWS
    std::unique_ptr<ServiceAdvertiser>  m_serviceAdvertiser;
#else
    std::unique_ptr<juce::NetworkServiceDiscovery::Advertiser>  m_serviceAdvertiser;
#endif
    std::unique_ptr<InterprocessConnectionServerImpl> m_networkServer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaProcessor)
};

} // namespace Mema

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

#include "MemaMessages.h"
#include "ProcessorDataAnalyzer.h"
#include "MemaPluginParameterInfo.h"
#include "../MemaProcessorEditor/MemaProcessorEditor.h"
#include "../MemaAppConfiguration.h"

#include <ServiceTopologyManager.h>


namespace Mema
{

class InterprocessConnectionImpl;
class InterprocessConnectionServerImpl;
class MemaChannelCommander;
class MemaInputCommander;
class MemaOutputCommander;
class MemaCrosspointCommander;
class MemaPluginCommander;
class MemaNetworkClientCommanderWrapper;


//==============================================================================
/**
 * @class ResizeableWindowWithTitleBarAndCloseCallback
 * @brief A resizable JUCE window with a title bar and close button that fires a callback on close.
 *
 * @details Used by `MemaProcessor` to host the loaded plugin's editor UI in a separate floating
 * window.  The `onClosed` callback allows `MemaProcessor` to clean up the plugin editor reference
 * when the user closes the window via the OS close button or via code.
 */
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
    std::function<void()> onClosed; ///< Invoked when the window is closed by the user or programmatically.
};

//==============================================================================
/**
 * @class PluginParameterInfosChangedMessage
 * @brief Internal JUCE message posted to the message thread when plugin parameter descriptors change.
 *
 * @details Posted by `MemaProcessor` (from a non-UI thread) when a plugin's parameter list or
 * remote-controllability settings have changed.  `MemaProcessor::handleMessage()` catches this
 * and broadcasts the updated `PluginParameterInfosMessage` to all connected Mema.Re clients.
 */
class PluginParameterInfosChangedMessage : public juce::Message
{
public:
    PluginParameterInfosChangedMessage() = default;
    virtual ~PluginParameterInfosChangedMessage() = default;
};

/**
 * @class MemaProcessor
 * @brief Core audio processor — owns the AudioDeviceManager, routing matrix, plugin host, and IPC server.
 *
 * @details MemaProcessor is the heart of the Mema tool.  It owns and coordinates all subsystems:
 *
 * ## Audio signal flow
 * ```
 * AudioDeviceManager ──► audioDeviceIOCallbackWithContext()
 *      │
 *      ├─► [if plugin pre-matrix] AudioPluginInstance::processBlock()
 *      │
 *      ├─► Input mutes  (m_inputMuteStates)
 *      ├─► Crosspoint matrix  (m_matrixCrosspointStates × m_matrixCrosspointValues)
 *      ├─► Output mutes  (m_outputMuteStates)
 *      │
 *      ├─► [if plugin post-matrix] AudioPluginInstance::processBlock()
 *      │
 *      ├─► Input ProcessorDataAnalyzer  → InputControlComponent (editor)
 *      └─► Output ProcessorDataAnalyzer → OutputControlComponent (editor) + TCP clients
 * ```
 *
 * ## Commander pattern
 * UI components that need to read or write routing state register themselves as commanders:
 * - `MemaInputCommander` / `MemaOutputCommander` — for per-channel mute control.
 * - `MemaCrosspointCommander` — for crosspoint enable/gain control.
 * - `MemaPluginCommander` — for plugin parameter control.
 *
 * Each `set*` method accepts an optional `sender` commander and `userId`.  When a change
 * originates from a network client (Mema.Re), the `userId` carries the client's connection-id
 * so that the update is echoed to all *other* clients but not reflected back to the originator.
 *
 * ## Network server
 * `InterprocessConnectionServerImpl` listens on port 55668.  On connect, MemaProcessor sends:
 * 1. `EnvironmentParametersMessage` — current palette style.
 * 2. `AnalyzerParametersMessage` — current sample rate and block size.
 * 3. `ReinitIOCountMessage` — current input/output channel counts.
 * 4. `ControlParametersMessage` — full routing-matrix state snapshot.
 * 5. `PluginParameterInfosMessage` — plugin name + parameter descriptors (if a plugin is loaded).
 *
 * Audio data is only streamed to clients that have subscribed via `DataTrafficTypeSelectionMessage`.
 *
 * ## Threading
 * - Audio I/O: `audioDeviceIOCallbackWithContext()` runs on the audio thread; protected by `m_audioDeviceIOCallbackLock`.
 * - Plugin processing: additionally protected by `m_pluginProcessingLock`.
 * - Network / message dispatch: `handleMessage()` runs on the JUCE message thread.
 * - Plugin parameter changes: `parameterValueChanged()` runs on whichever thread the plugin calls it from; posted to the message thread.
 *
 * @see MemaMessages.h — all TCP message types.
 * @see ProcessorDataAnalyzer — level/spectrum analysis fed by the audio callback.
 * @see MemaNetworkClientCommanderWrapper — bridges incoming `ControlParametersMessage` data to the commander pattern.
 */
class MemaProcessor : public juce::AudioProcessor,
    public juce::AudioIODeviceCallback,
    public juce::MessageListener,
    public juce::ChangeListener,
    public juce::AudioProcessorParameter::Listener,
    public MemaAppConfiguration::XmlConfigurableElement
{
public:
    /**
     * @brief Constructs the processor, optionally restoring state from XML.
     * @param stateXml Pointer to a previously serialised `<PROCESSORCONFIG>` XML element,
     *                 or `nullptr` to start with default (unity gain) crosspoint values.
     */
    MemaProcessor(XmlElement* stateXml);
    ~MemaProcessor();

    //==============================================================================
    /**
     * @brief Registers a listener to receive input-channel level/spectrum data from the input analyzer.
     * @param listener The listener to add; must remain valid until `removeInputListener()` is called.
     */
    void addInputListener(ProcessorDataAnalyzer::Listener* listener);
    /** @brief Unregisters a previously added input analyzer listener. @param listener The listener to remove. */
    void removeInputListener(ProcessorDataAnalyzer::Listener* listener);
    /**
     * @brief Registers a listener to receive output-channel level/spectrum data from the output analyzer.
     * @param listener The listener to add.
     */
    void addOutputListener(ProcessorDataAnalyzer::Listener* listener);
    /** @brief Unregisters a previously added output analyzer listener. @param listener The listener to remove. */
    void removeOutputListener(ProcessorDataAnalyzer::Listener* listener);

    //==============================================================================
    /**
     * @brief Adds an input commander and immediately pushes the current mute states to it.
     * @param commander The commander to register.  Ownership stays with the caller.
     */
    void addInputCommander(MemaInputCommander* commander);
    /**
     * @brief Pushes the current input mute states to a commander that was already registered.
     * @details Called after a commander has been constructed and wired up but needs its initial
     *          state (e.g. after a configuration reload).
     * @param commander The already-registered commander to initialise.
     */
    void initializeInputCommander(MemaInputCommander* commander);
    /** @brief Removes a previously registered input commander. @param commander The commander to remove. */
    void removeInputCommander(MemaInputCommander* commander);

    /**
     * @brief Adds an output commander and immediately pushes the current mute states to it.
     * @param commander The commander to register.
     */
    void addOutputCommander(MemaOutputCommander* commander);
    /** @brief Pushes the current output mute states to an already-registered commander. @param commander The commander to initialise. */
    void initializeOutputCommander(MemaOutputCommander* commander);
    /** @brief Removes a previously registered output commander. @param comander The commander to remove. */
    void removeOutputCommander(MemaOutputCommander* comander);

    /**
     * @brief Adds a crosspoint commander and immediately pushes the full crosspoint state to it.
     * @param commander The commander to register.
     */
    void addCrosspointCommander(MemaCrosspointCommander* commander);
    /** @brief Pushes the current crosspoint enable/gain matrix to an already-registered commander. @param commander The commander to initialise. */
    void initializeCrosspointCommander(MemaCrosspointCommander* commander);
    /** @brief Removes a previously registered crosspoint commander. @param comander The commander to remove. */
    void removeCrosspointCommander(MemaCrosspointCommander* comander);

    /**
     * @brief Adds a plugin commander and immediately pushes the current parameter infos and values.
     * @param commander The commander to register.
     */
    void addPluginCommander(MemaPluginCommander* commander);
    /** @brief Pushes the current plugin parameter descriptors and values to an already-registered commander. @param commander The commander to initialise. */
    void initializePluginCommander(MemaPluginCommander* commander);
    /** @brief Removes a previously registered plugin commander. @param commander The commander to remove. */
    void removePluginCommander(MemaPluginCommander* commander);

    /** @brief Forces all registered commanders to re-synchronise with the current processor state. */
    void updateCommanders();

    //==============================================================================
    /**
     * @brief Returns the mute state of a specific input channel.
     * @param channelNumber 1-based input channel index.
     * @return `true` if the channel is muted (audio is silenced before the crosspoint matrix).
     */
    bool getInputMuteState(std::uint16_t channelNumber);
    /**
     * @brief Sets the mute state of an input channel and notifies all commanders except the sender.
     * @param channelNumber 1-based input channel index.
     * @param muted         `true` to mute, `false` to unmute.
     * @param sender        The commander that triggered the change, or `nullptr` if the change is internal.
     *                      The sender is skipped when broadcasting the update to other commanders.
     * @param userId        Connection-id of the originating TCP client, used for echo-suppression (-1 = local).
     */
    void setInputMuteState(std::uint16_t channelNumber, bool muted, MemaChannelCommander* sender = nullptr, int userId = -1);

    /**
     * @brief Returns whether a specific crosspoint node is enabled (routing active).
     * @param inputNumber  1-based input channel index.
     * @param outputNumber 1-based output channel index.
     * @return `true` if the crosspoint is enabled and audio flows from that input to that output.
     */
    bool getMatrixCrosspointEnabledValue(std::uint16_t inputNumber, std::uint16_t outputNumber);
    /**
     * @brief Enables or disables a crosspoint routing node.
     * @param inputNumber  1-based input channel index.
     * @param outputNumber 1-based output channel index.
     * @param enabled      `true` to route audio, `false` to silence the node.
     * @param sender       Commander that triggered the change, or `nullptr`.
     * @param userId       Originating TCP client connection-id for echo-suppression (-1 = local).
     */
    void setMatrixCrosspointEnabledValue(std::uint16_t inputNumber, std::uint16_t outputNumber, bool enabled, MemaChannelCommander* sender = nullptr, int userId = -1);

    /**
     * @brief Returns the linear gain factor of a crosspoint node.
     * @param inputNumber  1-based input channel index.
     * @param outputNumber 1-based output channel index.
     * @return Linear gain in [0, 1].  Note: 1.0 = unity gain, 0.0 = silence.
     */
    float getMatrixCrosspointFactorValue(std::uint16_t inputNumber, std::uint16_t outputNumber);
    /**
     * @brief Sets the linear gain factor of a crosspoint node.
     * @param inputNumber  1-based input channel index.
     * @param outputNumber 1-based output channel index.
     * @param factor       Linear gain in [0, 1].
     * @param sender       Commander that triggered the change, or `nullptr`.
     * @param userId       Originating TCP client connection-id for echo-suppression (-1 = local).
     */
    void setMatrixCrosspointFactorValue(std::uint16_t inputNumber, std::uint16_t outputNumber, float factor, MemaChannelCommander* sender = nullptr, int userId = -1);

    /**
     * @brief Returns the current normalised value of a hosted plugin parameter.
     * @param pluginParameterIndex Zero-based parameter index within the plugin's parameter list.
     * @return Normalised value in [0, 1], or 0.0 if no plugin is loaded or the index is out of range.
     */
    float getPluginParameterValue(std::uint16_t pluginParameterIndex) const;
    /**
     * @brief Sets a hosted plugin parameter to a normalised value.
     * @param pluginParameterIndex Zero-based parameter index.
     * @param id                   Stable string ID used for cross-session safety.
     * @param normalizedValue      Normalised value in [0, 1].
     * @param sender               Commander that triggered the change, or `nullptr`.
     * @param userId               Originating TCP client connection-id for echo-suppression (-1 = local).
     */
    void setPluginParameterValue(std::uint16_t pluginParameterIndex, std::string id, float normalizedValue, MemaPluginCommander* sender = nullptr, int userId = -1);

    /**
     * @brief Returns the mute state of a specific output channel.
     * @param channelNumber 1-based output channel index.
     * @return `true` if the channel is muted (audio is silenced after the crosspoint matrix).
     */
    bool getOutputMuteState(std::uint16_t channelNumber);
    /**
     * @brief Sets the mute state of an output channel and notifies all commanders except the sender.
     * @param channelNumber 1-based output channel index.
     * @param muted         `true` to mute, `false` to unmute.
     * @param sender        Commander that triggered the change, or `nullptr`.
     * @param userId        Originating TCP client connection-id for echo-suppression (-1 = local).
     */
    void setOutputMuteState(std::uint16_t channelNumber, bool muted, MemaChannelCommander* sender = nullptr, int userId = -1);

    /**
     * @brief Resizes all internal routing structures for a new input/output channel count.
     * @details Called when the audio device reports a change in its channel layout.  Reinitialises
     *          `m_matrixCrosspointStates`, `m_matrixCrosspointValues`, mute maps, and the
     *          commander lists, then broadcasts a `ReinitIOCountMessage` to all connected clients.
     * @param inputChannelCount  New number of active input channels.
     * @param outputChannelCount New number of active output channels.
     */
    void setChannelCounts(std::uint16_t inputChannelCount, std::uint16_t outputChannelCount);

    //==============================================================================
    /**
     * @brief Loads and instantiates a plugin from the given description.
     * @details Scans the system for the matching plugin binary, creates an `AudioPluginInstance`,
     *          prepares it for playback, and registers MemaProcessor as a parameter listener.
     *          Triggers `onPluginSet` on success and broadcasts a `PluginParameterInfosMessage`.
     * @param pluginDescription The JUCE plugin description (obtained from a plugin scan).
     * @return `true` on success, `false` if the plugin could not be loaded.
     */
    bool setPlugin(const juce::PluginDescription& pluginDescription);
    /** @brief Returns the JUCE description of the currently loaded plugin. */
    juce::PluginDescription getPluginDescription();
    /** @brief Enables or disables plugin processing without unloading the plugin instance. @param enabled Pass `true` to enable, `false` to bypass. */
    void setPluginEnabledState(bool enabled);
    /** @brief Returns `true` when a plugin is loaded and its processing is enabled. */
    bool isPluginEnabled();
    /**
     * @brief Selects whether the plugin processes audio before or after the crosspoint matrix.
     * @param post `true` = post-matrix (plugin sees the fully routed mix); `false` = pre-matrix (plugin processes raw inputs).
     */
    void setPluginPrePostState(bool post);
    /** @brief Returns `true` when the plugin is inserted post-matrix. */
    bool isPluginPost();
    /** @brief Unloads the hosted plugin, closes its editor window, and resets all plugin commander state. */
    void clearPlugin();
    /** @brief Opens (or raises) the plugin's editor UI in a floating `ResizeableWindowWithTitleBarAndCloseCallback` window. */
    void openPluginEditor();
    /** @brief Closes the plugin editor window. @param deleteEditorWindow If `true`, also deletes the window object; pass `false` when the window is closing itself. */
    void closePluginEditor(bool deleteEditorWindow = true);
    std::function<void(const juce::PluginDescription&)> onPluginSet; ///< Invoked on the message thread after a new plugin has been successfully loaded.
    // Parameter management
    /** @brief Returns a mutable reference to the loaded plugin's parameter descriptor list. @return An empty vector if no plugin is loaded. */
    std::vector<PluginParameterInfo>& getPluginParameterInfos();
    /**
     * @brief Marks a plugin parameter as remotely controllable (or not) and sets its control widget type.
     * @param pluginParameterIndex Zero-based parameter index.
     * @param remoteControllable   `true` to expose this parameter in Mema.Re's plugin control panel.
     * @param type                 Control widget type (Continuous slider, Discrete combo box, or Toggle button).
     * @param steps                Number of discrete steps for `Discrete` type parameters.
     */
    void setPluginParameterRemoteControlInfos(int pluginParameterIndex, bool remoteControllable, ParameterControlType type, int steps);
    /**
     * @brief Sets the display order in which plugin parameters are presented to Mema.Re clients.
     * @param order Vector of zero-based parameter indices in the desired display order.
     *              Must be a permutation of all parameter indices.  Persisted to XML config.
     */
    void setPluginParameterDisplayOrder(const std::vector<int>& order);
    /** @brief Returns the current display order vector, or an empty vector if none has been set. */
    const std::vector<int>& getPluginParameterDisplayOrder() const;
    /** @brief Returns `true` if the given parameter is flagged as remotely controllable. @param parameterIndex Zero-based parameter index. */
    bool isPluginParameterRemoteControllable(int parameterIndex);
    /** @brief Returns a pointer to the underlying JUCE `AudioProcessorParameter` at the given index. @param parameterIndex Zero-based index. @return `nullptr` if no plugin is loaded or the index is out of range. */
    juce::AudioProcessorParameter* getPluginParameter(int parameterIndex) const;
    std::function<void(int pluginParameterIndex, float newValue)> onPluginParameterChanged; ///< Fired (on the message thread) when a hosted plugin parameter value changes; receives the zero-based index and new normalised value.
    std::function<void()> onPluginParameterInfosChanged; ///< Fired when the set of exposed plugin parameters changes (plugin load/unload or controllability settings change).
    std::function<void(bool enabled, bool post)> onPluginProcessingStateChanged; ///< Fired when the plugin enabled or pre/post state changes externally (e.g. from a Mema.Re client).

    //==============================================================================
    /** @brief Returns a raw pointer to the JUCE AudioDeviceManager. Used by the audio-setup UI component. */
    AudioDeviceManager* getDeviceManager();

    //==============================================================================
    /**
     * @brief Returns per-client network health metrics.
     * @return A map of {connectionId → {bytesPerSecond, isConnected}} for all currently connected TCP clients.
     *         Used by `MemaUIComponent` to update the network health bar.
     */
    std::map<int, std::pair<double, bool>> getNetworkHealth();

    //==============================================================================
    /** @brief Returns the most recent multicast service topology snapshot from `ServiceTopologyManager`. */
    JUCEAppBasics::SessionServiceTopology getDiscoveredServicesTopology();

    //==============================================================================
    /** @brief Returns the processor name ("Mema"). */
    const String getName() const override;
    /**
     * @brief Called by the JUCE audio engine before playback starts.
     * @details Broadcasts `AnalyzerParametersMessage` to all connected clients with the new
     *          sample rate and block size.
     * @param sampleRate                      New sample rate in Hz.
     * @param maximumExpectedSamplesPerBlock  Maximum number of samples per `processBlock()` call.
     */
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) override;
    /** @brief Called when playback stops; releases audio processing resources. */
    void releaseResources() override;
    /**
     * @brief Standard JUCE `AudioProcessor` entry point — not used for live audio.
     * @details `MemaProcessor` drives audio through `audioDeviceIOCallbackWithContext()` directly,
     *          not via the AudioProcessor plugin host path.  This override satisfies the interface
     *          contract but is otherwise a no-op.
     */
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
    /**
     * @brief Hot audio callback — implements the complete Mema signal chain.
     * @details This method runs on the dedicated audio thread at every audio device block:
     * 1. Copies device input channels into `m_processorChannels`.
     * 2. Optionally passes them through the hosted plugin (pre-matrix mode).
     * 3. Applies per-channel input mutes.
     * 4. Applies the crosspoint gain matrix (input × output).
     * 5. Applies per-channel output mutes.
     * 6. Optionally passes the result through the hosted plugin (post-matrix mode).
     * 7. Writes the result to the device output channels.
     * 8. Feeds both pre- and post-matrix buffers into the respective `ProcessorDataAnalyzer` instances.
     * 9. Serialises and sends `AudioInputBufferMessage` / `AudioOutputBufferMessage` to subscribed clients.
     * @note Protected by `m_audioDeviceIOCallbackLock` to prevent concurrent plugin load/unload.
     */
    void audioDeviceIOCallbackWithContext(const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const AudioIODeviceCallbackContext& context) override;

    /**
     * @brief Called when the audio device is about to start streaming.
     * @details Broadcasts `AnalyzerParametersMessage` and `ReinitIOCountMessage` to all connected clients.
     * @param device The audio device that is starting.
     */
    void audioDeviceAboutToStart(AudioIODevice* device) override;
    /** @brief Called when the audio device stops; notifies analyzers to clear their state. */
    void audioDeviceStopped() override;

    //==============================================================================
    /** @brief Receives notifications from the `AudioDeviceManager` when the device configuration changes. @param source The broadcaster that posted the change. */
    void changeListenerCallback(ChangeBroadcaster* source) override;

    //==============================================================================
    /**
     * @brief Dispatches JUCE messages posted to the message thread.
     * @details Handles:
     * - `ControlParametersMessage` — applies remote mute/crosspoint changes from Mema.Re.
     * - `PluginParameterValueMessage` — applies a single remote plugin parameter change.
     * - `PluginParameterInfosChangedMessage` — broadcasts updated parameter descriptors to clients.
     * @param message The message to handle.
     */
    void handleMessage(const Message& message) override;

    //==============================================================================
    /**
     * @brief Called by the hosted plugin when a parameter value changes.
     * @details Posts a message to the JUCE message thread so that `onPluginParameterChanged`
     *          is always fired on the UI thread.
     * @param parameterIndex Zero-based index of the changed parameter.
     * @param newValue       New normalised value in [0, 1].
     */
    void parameterValueChanged(int parameterIndex, float newValue) override;
    /** @brief Called by the hosted plugin when a gesture (e.g. mouse drag) starts or ends. @param parameterIndex Zero-based parameter index. @param gestureIsStarting `true` when the gesture begins. */
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    //==============================================================================
    /** @brief Serialises the current processor state (mutes, crosspoints, plugin settings) to XML. @return A new `<PROCESSORCONFIG>` XmlElement, or `nullptr` on failure. */
    std::unique_ptr<XmlElement> createStateXml() override;
    /** @brief Restores the processor state from a previously serialised `<PROCESSORCONFIG>` XmlElement. @param stateXml Pointer to the XML element. @return `true` on success. */
    bool setStateXml(XmlElement* stateXml) override;

    //==============================================================================
    /** @brief Called when the OS look-and-feel or palette changes; broadcasts `EnvironmentParametersMessage` to all connected clients. */
    void environmentChanged();

    /**
     * @brief Forces a full re-broadcast of device parameters and routing state to all connected clients.
     * @details Sends `AnalyzerParametersMessage`, `ReinitIOCountMessage`, `ControlParametersMessage`,
     *          and (if applicable) `PluginParameterInfosMessage` to every connected TCP client.
     *          Called after a device reconfiguration or configuration reload.
     */
    void triggerIOUpdate();

    //==============================================================================
    /**
     * @brief Updates the set of message types a specific TCP client has subscribed to receive.
     * @details Called by `InterprocessConnectionServerImpl` when a `DataTrafficTypeSelectionMessage`
     *          arrives from a client.  Only message types listed in `trafficTypes` will subsequently
     *          be forwarded to that connection.
     * @param trafficTypes The complete subscription list replacing any previous subscription.
     * @param connectionId The unique ID of the TCP client connection.
     */
    void setTrafficTypesForConnectionId(const std::vector<SerializableMessage::SerializableMessageType>& trafficTypes, int connectionId);

    //==============================================================================
    static constexpr int s_maxChannelCount = 64;    ///< Maximum number of input or output channels supported by the routing matrix.
    static constexpr int s_maxNumSamples = 1024;    ///< Maximum audio block size in samples.

    static constexpr int s_minInputsCount = 1;      ///< Minimum number of input channels (always at least 1).
    static constexpr int s_minOutputsCount = 1;     ///< Minimum number of output channels (always at least 1).

    //==============================================================================
    /** @brief Returns `true` when a deferred XML configuration dump has been scheduled. */
    bool isTimedConfigurationDumpPending() { return m_timedConfigurationDumpPending; };
    /** @brief Schedules a deferred XML configuration dump (called on state change to avoid excessive disk I/O). */
    void setTimedConfigurationDumpPending() { m_timedConfigurationDumpPending = true; };
    /** @brief Clears the deferred dump flag after the dump has been performed. */
    void resetTimedConfigurationDumpPending() { m_timedConfigurationDumpPending = false; };

    //==============================================================================
    /** @brief Resets all crosspoint gains to 1.0 (unity) and enables all crosspoints. Used when creating a default configuration. */
    void initializeCtrlValuesToUnity();

protected:
    //==============================================================================
    void initializeCtrlValues(int inputCount, int outputCount);
    void initializeCtrlValuesToUnity(int inputCount, int outputCount);

private:
    //==============================================================================
    void sendMessageToClients(const MemoryBlock& messageMemoryBlock, const std::vector<int>& sendIds);

    //==============================================================================
    /**
     * @brief Reconfigures the loaded plugin's channel layout and calls prepareToPlay for the current pre/post position.
     * @details Pre-matrix: plugin is sized to inputChannelCount × inputChannelCount so the routing matrix
     *          can widen or narrow to the output count afterwards.
     *          Post-matrix: plugin is sized to outputChannelCount × outputChannelCount.
     * @note Must be called under m_pluginProcessingLock.
     */
    void configurePluginForCurrentPosition();

    //==============================================================================
    juce::String    m_Name; ///< Processor name string returned by getName().

    //==============================================================================
    juce::CriticalSection   m_audioDeviceIOCallbackLock; ///< Mutex protecting the audio callback from concurrent plugin load/unload operations.

    float** m_processorChannels; ///< Raw float channel pointer array reused across audio callbacks to avoid per-block allocation.

    //==============================================================================
    std::unique_ptr<AudioDeviceManager> m_deviceManager; ///< JUCE AudioDeviceManager owning the audio hardware I/O.

    //==============================================================================
    std::unique_ptr<ProcessorDataAnalyzer>  m_inputDataAnalyzer; ///< Analyses pre-matrix input audio for level and spectrum data.
    std::unique_ptr<ProcessorDataAnalyzer>  m_outputDataAnalyzer; ///< Analyses post-matrix output audio for level and spectrum data.

    //==============================================================================
    std::vector<MemaInputCommander*>    m_inputCommanders; ///< All registered input-channel commander objects (UI + network).
    std::vector<MemaOutputCommander*>   m_outputCommanders; ///< All registered output-channel commander objects.
    std::vector<MemaCrosspointCommander*>   m_crosspointCommanders; ///< All registered crosspoint commander objects.
    std::vector<MemaPluginCommander*>   m_pluginCommanders; ///< All registered plugin parameter commander objects.

    //==============================================================================
    std::map<std::uint16_t, bool> m_inputMuteStates; ///< Per-input mute state (1-based channel index → muted).
    std::map<std::uint16_t, bool> m_outputMuteStates; ///< Per-output mute state.

    //==============================================================================
    int m_inputChannelCount{ 1 }; ///< Current number of active input channels.
    int m_outputChannelCount{ 1 }; ///< Current number of active output channels.

    //==============================================================================
    std::map<std::uint16_t, std::map<std::uint16_t, bool>>  m_matrixCrosspointStates; ///< Crosspoint enable matrix [in][out] → bool.
    std::map<std::uint16_t, std::map<std::uint16_t, float>>  m_matrixCrosspointValues; ///< Crosspoint linear gain matrix [in][out] → float.

    //==============================================================================
    std::unique_ptr<MemaProcessorEditor>  m_processorEditor; ///< The MemaProcessorEditor shown inside MemaUIComponent.

    //==============================================================================
    juce::CriticalSection                                           m_pluginProcessingLock; ///< Mutex protecting plugin load/unload from the audio thread.
    std::unique_ptr<juce::AudioPluginInstance>                      m_pluginInstance; ///< The hosted AudioPluginInstance (null if no plugin is loaded).
    bool                                                            m_pluginEnabled = false; ///< Whether plugin processing is active (false = bypass).
    bool                                                            m_pluginPost = false; ///< True = plugin inserted post-matrix; false = pre-matrix.
    int                                                             m_pluginConfiguredChannelCount{ 0 }; ///< Actual channel count the plugin was prepared with after bus layout negotiation.
    std::unique_ptr<ResizeableWindowWithTitleBarAndCloseCallback>   m_pluginEditorWindow; ///< Floating window hosting the plugin's editor UI.
    std::vector<PluginParameterInfo>                                m_pluginParameterInfos; ///< Cached parameter descriptor list for the loaded plugin.
    std::vector<int>                                                m_pluginParameterDisplayOrder; ///< User-defined display order: each element is a parameter index. Empty = natural order.

    //==============================================================================
    std::unique_ptr<juce::XmlElement> m_lastAppliedDeviceConfigXml; ///< Snapshot of the DEVCONFIG XML from the most recent successful audio device initialisation. Used to suppress redundant re-inits when only non-audio settings change.

    //==============================================================================
    std::unique_ptr<JUCEAppBasics::ServiceTopologyManager>  m_serviceTopologyManager; ///< Manages multicast service announcements so Mema.Mo/Re can discover this instance.
    std::shared_ptr<InterprocessConnectionServerImpl> m_networkServer; ///< TCP server listening on port 55668 for Mema.Mo and Mema.Re connections.
    std::unique_ptr<MemaNetworkClientCommanderWrapper> m_networkCommanderWrapper; ///< Bridges inbound ControlParametersMessage data into the commander pattern.
    std::map<int, std::vector<SerializableMessage::SerializableMessageType>> m_trafficTypesPerConnection; ///< Per-client subscription map: connectionId → list of subscribed SerializableMessageType values.

    std::unique_ptr<juce::TimedCallback>   m_timedConfigurationDumper; ///< Periodic callback that flushes pending XML configuration dumps to disk.
    bool    m_timedConfigurationDumpPending = false; ///< True when a configuration dump has been scheduled but not yet written.

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaProcessor)
};

} // namespace Mema

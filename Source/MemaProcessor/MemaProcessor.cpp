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

#include "MemaProcessor.h"

#include "InterprocessConnection.h"
#include "MemaCommanders.h"
#include "MemaServiceData.h"
#include "../MemaAppConfiguration.h"

#include <CustomLookAndFeel.h>

namespace Mema
{


#if JUCE_WINDOWS
/** Copy of juce::NetworkServiceDiscovery::Advertiser, simply because the underlying juce::IPAddress::getInterfaceBroadcastAddress is not implemented on windows and therefor the advertisement functionality not available here. */
struct ServiceAdvertiser : private juce::Thread
{
	ServiceAdvertiser(const String& serviceTypeUID,
		const String& serviceDescription,
		int broadcastPort,
		int connectionPort,
		RelativeTime minTimeBetweenBroadcasts = RelativeTime::seconds(1.5))
		: Thread(juce::JUCEApplication::getInstance()->getApplicationName() + ": Discovery_broadcast"),
		message(serviceTypeUID), broadcastPort(broadcastPort),
		minInterval(minTimeBetweenBroadcasts)
	{
		DBG("!!! Using juce::NetworkServiceDiscovery::Advertiser clone 'ServiceAdvertiser' just because juce::IPAddress::getInterfaceBroadcastAddress implementation is missing on windows. Rework required as soon as this changes. !!!");

		message.setAttribute("id", Uuid().toString());
		message.setAttribute("name", serviceDescription);
		message.setAttribute("address", String());
		message.setAttribute("port", connectionPort);

		startThread(Priority::background);
	};
	~ServiceAdvertiser() override {
		stopThread(2000);
		socket.shutdown();
	};

private:
	XmlElement message;
	const int broadcastPort;
	const RelativeTime minInterval;
	DatagramSocket socket{ true };

	IPAddress getInterfaceBroadcastAddress(const IPAddress& address)
	{
		if (address.isIPv6)
			// TODO
			return {};

		String broadcastAddress = address.toString().upToLastOccurrenceOf(".", true, false) + "255";
		return IPAddress(broadcastAddress);
	};
	void run() override
	{
		if (!socket.bindToPort(0))
		{
			jassertfalse;
			return;
		}

		while (!threadShouldExit())
		{
			sendBroadcast();
			wait((int)minInterval.inMilliseconds());
		}
	};
	void sendBroadcast()
	{
		static IPAddress local = IPAddress::local();

		for (auto& address : IPAddress::getAllAddresses())
		{
			if (address == local)
				continue;

			message.setAttribute("address", address.toString());

			auto broadcastAddress = getInterfaceBroadcastAddress(address);
			auto data = message.toString(XmlElement::TextFormat().singleLine().withoutHeader());

			socket.write(broadcastAddress.toString(), broadcastPort, data.toRawUTF8(), (int)data.getNumBytesAsUTF8());
		}
	};
};
#endif

//==============================================================================
class MemaNetworkClientCommanderWrapper : public MemaInputCommander, public MemaOutputCommander, public MemaCrosspointCommander
{
public:
	void setInputMute(std::uint16_t channel, bool muteState) override
	{
		if (m_networkServer && m_networkServer->hasActiveConnections())
		{
			std::map<std::uint16_t, bool> inputMuteStates;
			std::map<std::uint16_t, bool> outputMuteStates;
			std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>> crosspointStates;

			inputMuteStates[channel] = muteState;

			m_networkServer->enqueueMessage(std::make_unique<ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates)->getSerializedMessage());
		}
	};

	void setOutputMute(std::uint16_t channel, bool muteState) override
	{
		if (m_networkServer && m_networkServer->hasActiveConnections())
		{
			std::map<std::uint16_t, bool> inputMuteStates;
			std::map<std::uint16_t, bool> outputMuteStates;
			std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>> crosspointStates;

			outputMuteStates[channel] = muteState;

			m_networkServer->enqueueMessage(std::make_unique<ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates)->getSerializedMessage());
		}
	};

	void setCrosspointEnabledValue(std::uint16_t input, std::uint16_t output, bool enabledState) override
	{
		if (m_networkServer && m_networkServer->hasActiveConnections())
		{
			std::map<std::uint16_t, bool> inputMuteStates;
			std::map<std::uint16_t, bool> outputMuteStates;
			std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>> crosspointStates;

			crosspointStates[input][output] = std::make_pair(enabledState, enabledState ? 1.0f : 0.0f);

			m_networkServer->enqueueMessage(std::make_unique<ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates)->getSerializedMessage());
		}
	};

	void setCrosspointFactorValue(std::uint16_t input, std::uint16_t output, float factor) override
	{
		if (m_networkServer && m_networkServer->hasActiveConnections())
		{
			std::map<std::uint16_t, bool> inputMuteStates;
			std::map<std::uint16_t, bool> outputMuteStates;
			std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>> crosspointStates;

			crosspointStates[input][output] = std::make_pair(1.0f == factor, factor);

			m_networkServer->enqueueMessage(std::make_unique<ControlParametersMessage>(inputMuteStates, outputMuteStates, crosspointStates)->getSerializedMessage());
		}
	};

	void setIOCount(std::uint16_t inputCount, std::uint16_t outputCount) override
	{
		if (m_networkServer && m_networkServer->hasActiveConnections())
		{
			m_networkServer->enqueueMessage(std::make_unique<ReinitIOCountMessage>(inputCount, outputCount)->getSerializedMessage());
		}
	};

	void setNetworkConnection(const std::shared_ptr<InterprocessConnectionServerImpl>& networkServer)
	{
		m_networkServer = networkServer;
	}

private:
	void setChannelCount(std::uint16_t channelCount) override { ignoreUnused(channelCount); };

private:
	std::shared_ptr<InterprocessConnectionServerImpl> m_networkServer;

};

//==============================================================================
MemaProcessor::MemaProcessor(XmlElement* stateXml) :
	juce::AudioProcessor()
{
#ifdef RUN_MESSAGE_TESTS
	runTests();
#endif

	// prepare max sized processing data buffer
	m_processorChannels = new float* [s_maxChannelCount];
	for (auto i = 0; i < s_maxChannelCount; i++)
	{
		m_processorChannels[i] = new float[s_maxNumSamples];
		for (auto j = 0; j < s_maxNumSamples; j++)
		{
			m_processorChannels[i][j] = 0.0f;
		}
	}

	m_inputDataAnalyzer = std::make_unique<ProcessorDataAnalyzer>();
	m_outputDataAnalyzer = std::make_unique<ProcessorDataAnalyzer>();

	m_deviceManager = std::make_unique<AudioDeviceManager>();
	m_deviceManager->addAudioCallback(this);
    m_deviceManager->addChangeListener(this);
	
	if (!setStateXml(stateXml))
	{
        setStateXml(nullptr); // call without actual xml config causes default init
		triggerConfigurationUpdate(false);
	}

	// init the announcement of this app instance as discoverable service
#if JUCE_WINDOWS
	m_serviceAdvertiser = std::make_unique<ServiceAdvertiser>(
		Mema::ServiceData::getServiceTypeUID(),
		Mema::ServiceData::getServiceDescription(),
		Mema::ServiceData::getBroadcastPort(),
		Mema::ServiceData::getConnectionPort());
#else
	m_serviceAdvertiser = std::make_unique<juce::NetworkServiceDiscovery::Advertiser>(
		Mema::ServiceData::getServiceTypeUID(), 
		Mema::ServiceData::getServiceDescription(),
		Mema::ServiceData::getBroadcastPort(),
		Mema::ServiceData::getConnectionPort());
#endif

	m_networkServer = std::make_shared<InterprocessConnectionServerImpl>();
	m_networkServer->beginWaitingForSocket(Mema::ServiceData::getConnectionPort());
    m_networkServer->onConnectionCreated = [=](int connectionId) {
        auto connection = dynamic_cast<InterprocessConnectionImpl*>(m_networkServer->getActiveConnection(connectionId).get());
        if (connection)
        {
			connection->onConnectionLost = [=](int connectionId) { DBG(juce::String(__FUNCTION__) << " connection " << connectionId << " lost");
				m_trafficTypesPerConnection.erase(connectionId);
			};
			connection->onConnectionMade = [=](int connectionId ) { DBG(juce::String(__FUNCTION__) << " connection " << connectionId << " made");
				m_trafficTypesPerConnection[connectionId].clear();
				if (m_networkServer && m_networkServer->hasActiveConnections())
				{
					auto success = true;
					success = success && m_networkServer->enqueueMessage(std::make_unique<AnalyzerParametersMessage>(int(getSampleRate()), getBlockSize())->getSerializedMessage());
					success = success && m_networkServer->enqueueMessage(std::make_unique<ReinitIOCountMessage>(m_inputChannelCount, m_outputChannelCount)->getSerializedMessage());
					success = success && m_networkServer->enqueueMessage(std::make_unique<EnvironmentParametersMessage>(juce::Desktop::getInstance().isDarkModeActive() ? JUCEAppBasics::CustomLookAndFeel::PS_Dark : JUCEAppBasics::CustomLookAndFeel::PS_Light)->getSerializedMessage());
					success = success && m_networkServer->enqueueMessage(std::make_unique<ControlParametersMessage>(m_inputMuteStates, m_outputMuteStates, m_matrixCrosspointValues)->getSerializedMessage());
					if (!success)
						m_networkServer->cleanupDeadConnections();
				}
			};
			connection->onMessageReceived = [=](int connectionId, const juce::MemoryBlock& message) {
				if (auto knownMessage = Mema::SerializableMessage::initFromMemoryBlock(message))
				{
					knownMessage->setId(connectionId);
					postMessage(knownMessage);
				}
				else
					DBG(juce::String(__FUNCTION__) + " ignoring unknown message");
			};
        }
    };
	m_networkCommanderWrapper = std::make_unique<MemaNetworkClientCommanderWrapper>();
	m_networkCommanderWrapper->setNetworkConnection(m_networkServer);
	addInputCommander(static_cast<MemaInputCommander*>(m_networkCommanderWrapper.get()));
	addOutputCommander(static_cast<MemaOutputCommander*>(m_networkCommanderWrapper.get()));
	addCrosspointCommander(static_cast<MemaCrosspointCommander*>(m_networkCommanderWrapper.get()));

	m_timedConfigurationDumper = std::make_unique<juce::TimedCallback>([=]() {
		if (isTimedConfigurationDumpPending())
		{
			auto config = JUCEAppBasics::AppConfigurationBase::getInstance();
			if (config != nullptr)
				config->triggerConfigurationDump(false);
			resetTimedConfigurationDumpPending();
		}
	});
	m_timedConfigurationDumper->startTimer(100);
}

MemaProcessor::~MemaProcessor()
{
	m_timedConfigurationDumper->stopTimer();
	if (isTimedConfigurationDumpPending())
	{
		auto config = JUCEAppBasics::AppConfigurationBase::getInstance();
		if (config != nullptr)
			config->triggerConfigurationDump(false);
		resetTimedConfigurationDumpPending();
	}

	m_networkServer->stop();

	m_deviceManager->removeAudioCallback(this);

	// cleanup processing data buffer
	for (auto i = 0; i < s_maxChannelCount; i++)
		delete[] m_processorChannels[i];
	delete[] m_processorChannels;
}

std::unique_ptr<juce::XmlElement> MemaProcessor::createStateXml()
{
	auto stateXml = std::make_unique<juce::XmlElement>(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::PROCESSORCONFIG));

	auto devConfElm = std::make_unique<juce::XmlElement>(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::DEVCONFIG));
	if (m_deviceManager)
		devConfElm->addChildElement(m_deviceManager->createStateXml().release());
	stateXml->addChildElement(devConfElm.release());

	auto plgConfElm = std::make_unique<juce::XmlElement>(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::PLUGINCONFIG));
	plgConfElm->setAttribute(MemaAppConfiguration::getAttributeName(MemaAppConfiguration::AttributeID::ENABLED), m_pluginEnabled ? 1 : 0);
	plgConfElm->setAttribute(MemaAppConfiguration::getAttributeName(MemaAppConfiguration::AttributeID::POST), m_pluginPost ? 1 : 0);
	if (m_pluginInstance)
	{
		plgConfElm->addChildElement(m_pluginInstance->getPluginDescription().createXml().release());

		juce::MemoryBlock destData;
		m_pluginInstance->getStateInformation(destData);
		plgConfElm->addTextElement(juce::Base64::toBase64(destData.getData(), destData.getSize()));
	}
	stateXml->addChildElement(plgConfElm.release());

	std::map<std::uint16_t, bool> inputMuteStates;
	std::map<std::uint16_t, bool> outputMuteStates;
	std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>> matrixCrosspointValues;
	{
		// copy the processing relevant variables to not block audio thread during all the xml handling
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		inputMuteStates = m_inputMuteStates;
		outputMuteStates = m_outputMuteStates;
		matrixCrosspointValues = m_matrixCrosspointValues;
	}

	auto inputMutesElm = std::make_unique<juce::XmlElement>(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::INPUTMUTES));
	juce::StringArray imutestatestr;
	for (auto const& mutestate : inputMuteStates)
		imutestatestr.add(juce::String(mutestate.first) + "," + juce::String(mutestate.second ? 1 : 0));
	inputMutesElm->addTextElement(imutestatestr.joinIntoString(";"));
	stateXml->addChildElement(inputMutesElm.release());

	auto outputMutesElm = std::make_unique<juce::XmlElement>(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::OUTPUTMUTES));
	juce::StringArray omutestatestr;
	for (auto const& mutestate : outputMuteStates)
		omutestatestr.add(juce::String(mutestate.first) + "," + juce::String(mutestate.second ? 1 : 0));
	outputMutesElm->addTextElement(omutestatestr.joinIntoString(";"));
	stateXml->addChildElement(outputMutesElm.release());

	auto crosspointGainsElm = std::make_unique<juce::XmlElement>(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::CROSSPOINTGAINS));
	juce::StringArray cgainstatestr;
	for (auto const& insKV : matrixCrosspointValues)
		for (auto const& outsKV : insKV.second)
			cgainstatestr.add(juce::String(insKV.first) + "," + juce::String(outsKV.first) + "," + juce::String(outsKV.second.first ? 1 : 0) + "," + juce::String(outsKV.second.second)); // "in,out,enabled,gain;"
	crosspointGainsElm->addTextElement(cgainstatestr.joinIntoString(";"));
	stateXml->addChildElement(crosspointGainsElm.release());

	return stateXml;
}

bool MemaProcessor::setStateXml(juce::XmlElement* stateXml)
{
	if (nullptr == stateXml || stateXml->getTagName() != MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::PROCESSORCONFIG))
	{
		jassertfalse;
		return false;
	}

    juce::XmlElement* deviceSetupXml = nullptr;
    auto devConfElm = stateXml->getChildByName(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::DEVCONFIG));
    if (nullptr != devConfElm)
        deviceSetupXml = devConfElm->getChildByName("DEVICESETUP");
    
	if (m_deviceManager)
	{
        // Hacky bit of device manager initialization:
        // We first intialize it to be able to get a valid device setup,
        // then initialize with a dummy xml config to trigger the internal xml structure being reset
        // and finally apply the original initialized device setup again to have the audio running correctly.
        // If we did not do so, either the internal xml would not be present as long as the first configuration change was made
        // and therefor no valid config file could be written by the application or the audio would not be running
        // on first start and manual config would be required.
        m_deviceManager->initialiseWithDefaultDevices(s_maxChannelCount, s_maxChannelCount);
        auto audioDeviceSetup = m_deviceManager->getAudioDeviceSetup();
		auto result = m_deviceManager->initialise(s_maxChannelCount, s_maxChannelCount, deviceSetupXml, true, {}, &audioDeviceSetup);
        if (result.isNotEmpty())
        {
            juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, juce::JUCEApplication::getInstance()->getApplicationName() + " device init failed", result);
            return false;
        }
        else
        {
#if JUCE_IOS
            if (audioDeviceSetup.bufferSize < 512)
                audioDeviceSetup.bufferSize = 512; // temp. workaround for iOS where buffersizes <512 lead to no sample data being delivered?
			m_deviceManager->setAudioDeviceSetup(audioDeviceSetup, true);
#endif
        }
	}
	else
		return false;


	auto plgConfElm = stateXml->getChildByName(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::PLUGINCONFIG));
	if (nullptr != plgConfElm)
	{
		setPluginEnabledState(plgConfElm->getBoolAttribute(MemaAppConfiguration::getAttributeName(MemaAppConfiguration::AttributeID::ENABLED)));
		setPluginPrePostState(plgConfElm->getBoolAttribute(MemaAppConfiguration::getAttributeName(MemaAppConfiguration::AttributeID::POST)));
		auto pluginDescriptionXml = plgConfElm->getChildByName("PLUGIN");
		if (nullptr != pluginDescriptionXml)
		{
			auto pluginDescription = juce::PluginDescription();
			pluginDescription.loadFromXml(*pluginDescriptionXml);
			setPlugin(pluginDescription);
			if (m_pluginInstance)
			{
				juce::MemoryOutputStream destDataStream;
				juce::Base64::convertFromBase64(destDataStream, pluginDescriptionXml->getAllSubText());
				m_pluginInstance->setStateInformation(destDataStream.getData(), int(destDataStream.getDataSize()));
			}
		}
	}

	std::map<std::uint16_t, bool> inputMuteStates;
	std::map<std::uint16_t, bool> outputMuteStates;
	std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>> matrixCrosspointValues;
	auto inputMutesElm = stateXml->getChildByName(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::INPUTMUTES));
	if (nullptr != inputMutesElm)
	{
		juce::StringArray imutestatestrs;
		imutestatestrs.addTokens(inputMutesElm->getAllSubText(), ";", "");
		for (auto const& imutestatestr : imutestatestrs)
		{
			juce::StringArray imutestatestra;
			imutestatestra.addTokens(imutestatestr, ",", "");
			jassert(2 == imutestatestra.size());
			if (2 == imutestatestra.size())
				inputMuteStates[std::uint16_t(imutestatestra[0].getIntValue())] = (1 == std::uint16_t(imutestatestra[1].getIntValue()));
		}
	}
	auto outputMutesElm = stateXml->getChildByName(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::OUTPUTMUTES));
	if (nullptr != outputMutesElm)
	{
		juce::StringArray omutestatestrs;
		omutestatestrs.addTokens(outputMutesElm->getAllSubText(), ";", "");
		for (auto const& omutestatestr : omutestatestrs)
		{
			juce::StringArray omutestatestra;
			omutestatestra.addTokens(omutestatestr, ",", "");
			jassert(2 == omutestatestra.size());
			if (2 == omutestatestra.size())
				outputMuteStates[std::uint16_t(omutestatestra[0].getIntValue())] = (1 == std::uint16_t(omutestatestra[1].getIntValue()));
		}
	}
	auto crosspointGainsElm = stateXml->getChildByName(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::CROSSPOINTGAINS));
	if (nullptr != crosspointGainsElm)
	{
		juce::StringArray cgainstatestrs;
		cgainstatestrs.addTokens(crosspointGainsElm->getAllSubText(), ";", "");
		for (auto const& cgainstatestr : cgainstatestrs)
		{
			juce::StringArray cgainstatestra;
			cgainstatestra.addTokens(cgainstatestr, ",", "");
			jassert(4 == cgainstatestra.size());
			if (4 == cgainstatestra.size())
				matrixCrosspointValues[std::uint16_t(cgainstatestra[0].getIntValue())][std::uint16_t(cgainstatestra[1].getIntValue())] = std::make_pair(1 == std::uint16_t(cgainstatestra[2].getIntValue()), cgainstatestra[3].getFloatValue());
		}
	}
	{
		// copy the processing relevant variables from temp vars here to not block audio thread during all the xml handling above
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		m_inputMuteStates = inputMuteStates;
		m_outputMuteStates = outputMuteStates;
		m_matrixCrosspointValues = matrixCrosspointValues;
	}

	return true;
}

void MemaProcessor::environmentChanged()
{
	auto paletteStyle = JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Dark;
	if (getActiveEditor())
		if (auto claf = dynamic_cast<JUCEAppBasics::CustomLookAndFeel*>(&getActiveEditor()->getLookAndFeel()))
			paletteStyle = claf->getPaletteStyle();

	postMessage(std::make_unique<EnvironmentParametersMessage>(paletteStyle).release());
}

void MemaProcessor::triggerIOUpdate()
{
	postMessage(std::make_unique<ReinitIOCountMessage>(m_inputChannelCount, m_outputChannelCount).release());
}

void MemaProcessor::addInputListener(ProcessorDataAnalyzer::Listener* listener)
{
	if (m_inputDataAnalyzer)
		m_inputDataAnalyzer->addListener(listener);
}

void MemaProcessor::removeInputListener(ProcessorDataAnalyzer::Listener* listener)
{
	if (m_inputDataAnalyzer)
		m_inputDataAnalyzer->removeListener(listener);
}

void MemaProcessor::addOutputListener(ProcessorDataAnalyzer::Listener* listener)
{
	if (m_outputDataAnalyzer)
		m_outputDataAnalyzer->addListener(listener);
}

void MemaProcessor::removeOutputListener(ProcessorDataAnalyzer::Listener* listener)
{
	if (m_outputDataAnalyzer)
		m_outputDataAnalyzer->removeListener(listener);
}

void MemaProcessor::addInputCommander(MemaInputCommander* commander)
{
	if (commander == nullptr)
		return;

	if (std::find(m_inputCommanders.begin(), m_inputCommanders.end(), commander) == m_inputCommanders.end())
	{
		initializeInputCommander(commander);

		m_inputCommanders.push_back(commander);
		commander->setInputMuteChangeCallback([=](MemaChannelCommander* sender, int channel, bool state) { return setInputMuteState(std::uint16_t(channel), state, sender); } );
	}
}

void MemaProcessor::initializeInputCommander(MemaInputCommander* commander)
{
	if (nullptr != commander)
	{
        const ScopedLock sl(m_audioDeviceIOCallbackLock);
        for (auto const& inputMuteStatesKV : m_inputMuteStates)
            commander->setInputMute(inputMuteStatesKV.first, inputMuteStatesKV.second);
	}
}

void MemaProcessor::removeInputCommander(MemaInputCommander* commander)
{
	if (commander == nullptr)
		return;

	auto existingInputCommander = std::find(m_inputCommanders.begin(), m_inputCommanders.end(), commander);
	if (existingInputCommander != m_inputCommanders.end())
		m_inputCommanders.erase(existingInputCommander);
}

void MemaProcessor::addOutputCommander(MemaOutputCommander* commander)
{
	if (commander == nullptr)
		return;

	if (std::find(m_outputCommanders.begin(), m_outputCommanders.end(), commander) == m_outputCommanders.end())
	{
		initializeOutputCommander(commander);

		m_outputCommanders.push_back(commander);
		commander->setOutputMuteChangeCallback([=](MemaChannelCommander* sender, int channel, bool state) { return setOutputMuteState(std::uint16_t(channel), state, sender); });
	}
}

void MemaProcessor::initializeOutputCommander(MemaOutputCommander* commander)
{
	if (nullptr != commander)
	{
        const ScopedLock sl(m_audioDeviceIOCallbackLock);
        for (auto const& outputMuteStatesKV : m_outputMuteStates)
            commander->setOutputMute(outputMuteStatesKV.first, outputMuteStatesKV.second);
	}
}

void MemaProcessor::removeOutputCommander(MemaOutputCommander* commander)
{
	if (commander == nullptr)
		return;

	auto existingOutputCommander = std::find(m_outputCommanders.begin(), m_outputCommanders.end(), commander);
	if (existingOutputCommander != m_outputCommanders.end())
		m_outputCommanders.erase(existingOutputCommander);
}

void MemaProcessor::addCrosspointCommander(MemaCrosspointCommander* commander)
{
	if (commander == nullptr)
		return;

	if (std::find(m_crosspointCommanders.begin(), m_crosspointCommanders.end(), commander) == m_crosspointCommanders.end())
	{
		initializeCrosspointCommander(commander);

		m_crosspointCommanders.push_back(commander);
		commander->setCrosspointEnabledChangeCallback([=](MemaChannelCommander* sender, int input, int output, bool state) { return setMatrixCrosspointEnabledValue(std::uint16_t(input), std::uint16_t(output), state, sender); });
		commander->setCrosspointFactorChangeCallback([=](MemaChannelCommander* sender, int input, int output, float factor) { return setMatrixCrosspointFactorValue(std::uint16_t(input), std::uint16_t(output), factor, sender); });
	}
}

void MemaProcessor::initializeCrosspointCommander(MemaCrosspointCommander* commander)
{
	if (nullptr != commander)
	{
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		for (auto const& matrixCrosspointValKV : m_matrixCrosspointValues)
		{
			for (auto const& matrixCrosspointValNodeKV : matrixCrosspointValKV.second)
			{
				auto& input = matrixCrosspointValKV.first;
				auto& output = matrixCrosspointValNodeKV.first;
				auto& val = matrixCrosspointValNodeKV.second;
				auto& enabled = val.first;
				auto& factor = val.second;
				commander->setCrosspointEnabledValue(input, output, enabled);
				commander->setCrosspointFactorValue(input, output, factor);
			}
		}
	}
}

void MemaProcessor::removeCrosspointCommander(MemaCrosspointCommander* commander)
{
	if (commander == nullptr)
		return;

	auto existingCrosspointCommander = std::find(m_crosspointCommanders.begin(), m_crosspointCommanders.end(), commander);
	if (existingCrosspointCommander != m_crosspointCommanders.end())
		m_crosspointCommanders.erase(existingCrosspointCommander);
}

void MemaProcessor::updateCommanders()
{
	for (auto const& ic : m_inputCommanders)
	{
		ic->setChannelCount(m_inputChannelCount > 0 ? std::uint16_t(m_inputChannelCount) : 0);
		initializeInputCommander(ic);
	}
	for (auto const& cc : m_crosspointCommanders)
	{
		cc->setIOCount(m_inputChannelCount > 0 ? std::uint16_t(m_inputChannelCount) : 0, m_outputChannelCount > 0 ? std::uint16_t(m_outputChannelCount) : 0);
		initializeCrosspointCommander(cc);
	}
	for (auto const& oc : m_outputCommanders)
	{
		oc->setChannelCount(m_outputChannelCount > 0 ? std::uint16_t(m_outputChannelCount) : 0);
		initializeOutputCommander(oc);
	}
}

bool MemaProcessor::getInputMuteState(std::uint16_t inputChannelNumber)
{
	jassert(inputChannelNumber > 0);
	const ScopedLock sl(m_audioDeviceIOCallbackLock);
	return m_inputMuteStates[inputChannelNumber];
}

void MemaProcessor::setInputMuteState(std::uint16_t inputChannelNumber, bool muted, MemaChannelCommander* sender)
{
	jassert(inputChannelNumber > 0);

	for (auto const& inputCommander : m_inputCommanders)
	{
		if (inputCommander != reinterpret_cast<MemaInputCommander*>(sender))
			inputCommander->setInputMute(inputChannelNumber, muted);
	}

	{
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		m_inputMuteStates[inputChannelNumber] = muted;
	}

	// sending to connected clients T.B.D.

	setTimedConfigurationDumpPending();
}

bool MemaProcessor::getMatrixCrosspointEnabledValue(std::uint16_t inputNumber, std::uint16_t outputNumber)
{
    jassert(inputNumber > 0 && outputNumber > 0);
	{
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		if (m_matrixCrosspointValues.count(inputNumber) != 0)
			if (m_matrixCrosspointValues.at(inputNumber).count(outputNumber) != 0)
				return m_matrixCrosspointValues.at(inputNumber).at(outputNumber).first;
	}
	jassertfalse;
	return false;
}

void MemaProcessor::setMatrixCrosspointEnabledValue(std::uint16_t inputNumber, std::uint16_t outputNumber, bool enabled, MemaChannelCommander* sender)
{
    jassert(inputNumber > 0 && outputNumber > 0);

    for (auto const& crosspointCommander : m_crosspointCommanders)
    {
        if (crosspointCommander != reinterpret_cast<MemaCrosspointCommander*>(sender))
            crosspointCommander->setCrosspointEnabledValue(inputNumber, outputNumber, enabled);
    }

	{
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		m_matrixCrosspointValues[inputNumber][outputNumber].first = enabled;
	}

	// sending to connected clients T.B.D.

	setTimedConfigurationDumpPending();
}

float MemaProcessor::getMatrixCrosspointFactorValue(std::uint16_t inputNumber, std::uint16_t outputNumber)
{
	jassert(inputNumber > 0 && outputNumber > 0);
	{
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		if (m_matrixCrosspointValues.count(inputNumber) != 0)
			if (m_matrixCrosspointValues.at(inputNumber).count(outputNumber) != 0)
				return m_matrixCrosspointValues.at(inputNumber).at(outputNumber).second;
	}
	jassertfalse;
	return 0.0f;
}

void MemaProcessor::setMatrixCrosspointFactorValue(std::uint16_t inputNumber, std::uint16_t outputNumber, float factor, MemaChannelCommander* sender)
{
	jassert(inputNumber > 0 && outputNumber > 0);

	for (auto const& crosspointCommander : m_crosspointCommanders)
	{
		if (crosspointCommander != reinterpret_cast<MemaCrosspointCommander*>(sender))
			crosspointCommander->setCrosspointFactorValue(inputNumber, outputNumber, factor);
	}

	{
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		m_matrixCrosspointValues[inputNumber][outputNumber].second = factor;
	}

	// sending to connected clients T.B.D.

	setTimedConfigurationDumpPending();
}

bool MemaProcessor::getOutputMuteState(std::uint16_t outputChannelNumber)
{
	jassert(outputChannelNumber > 0);
	const ScopedLock sl(m_audioDeviceIOCallbackLock);
	return m_outputMuteStates[outputChannelNumber];
}

void MemaProcessor::setOutputMuteState(std::uint16_t outputChannelNumber, bool muted, MemaChannelCommander* sender)
{
	jassert(outputChannelNumber > 0);

	for (auto const& outputCommander : m_outputCommanders)
	{
		if (outputCommander != reinterpret_cast<MemaOutputCommander*>(sender))
			outputCommander->setOutputMute(outputChannelNumber, muted);
	}

	{
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		m_outputMuteStates[outputChannelNumber] = muted;
	}

	// sending to connected clients T.B.D.

	setTimedConfigurationDumpPending();
}

void MemaProcessor::setChannelCounts(std::uint16_t inputChannelCount, std::uint16_t outputChannelCount)
{
    auto reinitRequired = false;
    if (m_inputChannelCount != inputChannelCount)
    {
        m_inputChannelCount = inputChannelCount;
        reinitRequired = true;

		// threadsafe locking in scope to access plugin
        {
            const ScopedLock sl(m_pluginProcessingLock);
			if (m_pluginInstance)
	            m_pluginInstance->setPlayConfigDetails(m_inputChannelCount, m_inputChannelCount, getSampleRate(), getBlockSize());
        }
    }
    if (m_outputChannelCount != outputChannelCount)
    {
        m_outputChannelCount = outputChannelCount;
        reinitRequired = true;
    }
    if (reinitRequired)
        postMessage(new ReinitIOCountMessage(m_inputChannelCount, m_outputChannelCount));
}

bool MemaProcessor::setPlugin(const juce::PluginDescription& pluginDescription)
{
	juce::AudioPluginFormatManager formatManager;
	formatManager.addDefaultFormats();
	auto registeredFormats = formatManager.getFormats();

	auto success = false;
	juce::String errorMessage = "Unsupported plug-in format.";

	for (auto const& format : registeredFormats)
	{
		if (format->getName() == pluginDescription.pluginFormatName)
		{
			closePluginEditor();

			// threadsafe locking in scope to access plugin
			{
				const ScopedLock sl(m_pluginProcessingLock);
				m_pluginInstance = format->createInstanceFromDescription(pluginDescription, getSampleRate(), getBlockSize(), errorMessage);
				if (m_pluginInstance)
                {
                    m_pluginInstance->setPlayConfigDetails(m_inputChannelCount, m_inputChannelCount, getSampleRate(), getBlockSize());
                    m_pluginInstance->prepareToPlay(getSampleRate(), getBlockSize());
                }
			}
			success = errorMessage.isEmpty();
			break;
		}
	}

	if (!success)
		juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Loading error", "Loading of the selected plug-in " + pluginDescription.name + " failed.\n" + errorMessage);
	else if (onPluginSet)
		onPluginSet(pluginDescription);

	triggerConfigurationUpdate(false);

	return success;
}

juce::PluginDescription MemaProcessor::getPluginDescription()
{
	if (m_pluginInstance)
		return m_pluginInstance->getPluginDescription();

	return {};
}

void MemaProcessor::setPluginEnabledState(bool enabled)
{
	// threadsafe locking in scope to access plugin enabled
	{
		const ScopedLock sl(m_pluginProcessingLock);
		m_pluginEnabled = enabled;
	}

	triggerConfigurationUpdate(false);
}

bool MemaProcessor::isPluginEnabled()
{
	return m_pluginEnabled;
}

void MemaProcessor::setPluginPrePostState(bool post)
{
	// threadsafe locking in scope to access plugin enabled
	{
		const ScopedLock sl(m_pluginProcessingLock);
		m_pluginPost = post;
	}

	triggerConfigurationUpdate(false);
}

bool MemaProcessor::isPluginPost()
{
	return m_pluginPost;
}

void MemaProcessor::clearPlugin()
{
	closePluginEditor();

	// threadsafe locking in scope to access plugin
	{
		const ScopedLock sl(m_pluginProcessingLock);
		m_pluginInstance.reset();
	}

	if (onPluginSet)
		onPluginSet(juce::PluginDescription());

	setPluginEnabledState(false);
}

void MemaProcessor::openPluginEditor()
{
	if (m_pluginInstance)
	{
		auto pluginEditorInstance = m_pluginInstance->createEditorIfNeeded();
		if (pluginEditorInstance && !m_pluginEditorWindow)
		{
			m_pluginEditorWindow = std::make_unique<ResizeableWindowWithTitleBarAndCloseCallback>(juce::JUCEApplication::getInstance()->getApplicationName() + " : " + m_pluginInstance->getName(), true);
			m_pluginEditorWindow->setResizable(false, false);
			m_pluginEditorWindow->setContentOwned(pluginEditorInstance, true);
			m_pluginEditorWindow->onClosed = [=]() { closePluginEditor(true); };
			m_pluginEditorWindow->setVisible(true);
		}
	}
}

void MemaProcessor::closePluginEditor(bool deleteEditorWindow)
{
	if (m_pluginInstance)
		std::unique_ptr<juce::AudioProcessorEditor>(m_pluginInstance->getActiveEditor()).reset();
	if (deleteEditorWindow)
		m_pluginEditorWindow.reset();
}

AudioDeviceManager* MemaProcessor::getDeviceManager()
{
	if (m_deviceManager)
		return m_deviceManager.get();
	else
		return nullptr;
}

std::map<int, std::pair<double, bool>> MemaProcessor::getNetworkHealth()
{
	if (m_networkServer)
		return m_networkServer->getListHealth();
	else
		return {};
}

//==============================================================================
const String MemaProcessor::getName() const
{
	return m_Name;
}

void MemaProcessor::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
	setRateAndBufferSizeDetails(sampleRate, maximumExpectedSamplesPerBlock);

	// threadsafe locking in scope to access plugin
	{
		const ScopedLock sl(m_pluginProcessingLock);
		if (m_pluginInstance)
			m_pluginInstance->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
	}

	if (m_inputDataAnalyzer)
		m_inputDataAnalyzer->initializeParameters(sampleRate, maximumExpectedSamplesPerBlock);
	if (m_outputDataAnalyzer)
		m_outputDataAnalyzer->initializeParameters(sampleRate, maximumExpectedSamplesPerBlock);

	postMessage(std::make_unique<AnalyzerParametersMessage>(int(sampleRate), maximumExpectedSamplesPerBlock).release());
}

void MemaProcessor::releaseResources()
{
	// threadsafe locking in scope to access plugin
	{
		const ScopedLock sl(m_pluginProcessingLock);
		if (m_pluginInstance)
			m_pluginInstance->releaseResources();
	}

	if (m_inputDataAnalyzer)
		m_inputDataAnalyzer->clearParameters();
	if (m_outputDataAnalyzer)
		m_outputDataAnalyzer->clearParameters();
}

void MemaProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	ignoreUnused(midiMessages);

	// the lock is currently gloablly taken in audioDeviceIOCallback which is calling this method
	//const ScopedLock sl(m_audioDeviceIOCallbackLock);

	auto reinitRequired = false;

	jassert(s_minInputsCount <= m_inputChannelCount);
	jassert(s_minOutputsCount <= m_outputChannelCount);

	if (m_inputChannelCount > m_inputMuteStates.size())
		reinitRequired = true;

	for (std::uint16_t input = 1; input <= m_inputChannelCount; input++)
	{
		if (m_inputMuteStates.count(input) != 0 && m_inputMuteStates.at(input))
		{
			auto channelIdx = input - 1;
			buffer.clear(channelIdx, 0, buffer.getNumSamples());
		}
	}

	// threadsafe locking in scope to access plugin - processing only takes place if NOT set to post matrix
	{
		const ScopedLock sl(m_pluginProcessingLock);
		if (m_pluginInstance && m_pluginEnabled && !m_pluginPost)
			m_pluginInstance->processBlock(buffer, midiMessages);
	}

	postMessage(std::make_unique<AudioInputBufferMessage>(buffer).release());

	// process data in buffer to be what shall be used as output
	juce::AudioBuffer<float> processedBuffer;
	processedBuffer.setSize(m_outputChannelCount, buffer.getNumSamples(), false, true, true);
	for (std::uint16_t inputIdx = 0; inputIdx < m_inputChannelCount; inputIdx++)
	{
		for (std::uint16_t outputIdx = 0; outputIdx < m_outputChannelCount; outputIdx++)
		{
			if (m_matrixCrosspointValues.count(inputIdx + 1) != 0)
			{
				if (m_matrixCrosspointValues[inputIdx + 1].count(outputIdx + 1) != 0)
				{
					auto& crosspointValues = m_matrixCrosspointValues.at(inputIdx + 1).at(outputIdx + 1);
					auto& enabled = crosspointValues.first;
					auto& factor = crosspointValues.second;
					auto gain = !enabled ? 0.0f : factor;
					processedBuffer.addFrom(outputIdx, 0, buffer.getReadPointer(inputIdx), buffer.getNumSamples(), gain);
				}
			}
		}
	}
	buffer.makeCopyOf(processedBuffer, true);

	if (m_outputChannelCount > m_outputMuteStates.size())
		reinitRequired = true;

	// threadsafe locking in scope to access plugin - processing only takes place if set to post matrix
	{
		const ScopedLock sl(m_pluginProcessingLock);
		if (m_pluginInstance && m_pluginEnabled && m_pluginPost)
			m_pluginInstance->processBlock(buffer, midiMessages);
	}

	for (std::uint16_t output = 1; output <= m_outputChannelCount; output++)
	{
		if (m_outputMuteStates.count(output) != 0 && m_outputMuteStates.at(output))
		{
			auto channelIdx = output - 1;
			buffer.clear(channelIdx, 0, buffer.getNumSamples());
		}
	}

	postMessage(std::make_unique<AudioOutputBufferMessage>(buffer).release());

	if (reinitRequired)
		postMessage(std::make_unique<ReinitIOCountMessage>(m_inputChannelCount, m_outputChannelCount).release());
}

void MemaProcessor::handleMessage(const Message& message)
{
	auto tId = SerializableMessage::SerializableMessageType::None;
	juce::MemoryBlock serializedMessageMemoryBlock;
	if (auto const epm = dynamic_cast<const EnvironmentParametersMessage*>(&message))
	{
		serializedMessageMemoryBlock = epm->getSerializedMessage();
		tId = epm->getType();
	}
	else if (auto const apm = dynamic_cast<const AnalyzerParametersMessage*>(&message))
	{
		serializedMessageMemoryBlock = apm->getSerializedMessage();
		tId = apm->getType();
	}
	else if (auto const iom = dynamic_cast<const ReinitIOCountMessage*> (&message))
	{
		auto inputCount = iom->getInputCount();
		jassert(inputCount > 0);
		auto outputCount = iom->getOutputCount();
		jassert(outputCount > 0);

		for (auto const& inputCommander : m_inputCommanders)
			inputCommander->setChannelCount(inputCount);
		
		for (auto const& outputCommander : m_outputCommanders)
			outputCommander->setChannelCount(outputCount);

		for (auto const& crosspointCommander : m_crosspointCommanders)
			crosspointCommander->setIOCount(inputCount, outputCount);

		initializeCtrlValues(iom->getInputCount(), iom->getOutputCount());

		serializedMessageMemoryBlock = iom->getSerializedMessage();

		tId = iom->getType();
	}
	else if (auto m = dynamic_cast<const AudioBufferMessage*> (&message))
	{
		if (m->getFlowDirection() == AudioBufferMessage::FlowDirection::Input && m_inputDataAnalyzer)
		{
			m_inputDataAnalyzer->analyzeData(m->getAudioBuffer());
		}
		else if (m->getFlowDirection() == AudioBufferMessage::FlowDirection::Output && m_outputDataAnalyzer)
		{
			m_outputDataAnalyzer->analyzeData(m->getAudioBuffer());
		}

		serializedMessageMemoryBlock = m->getSerializedMessage();

		tId = m->getType();
	}
	else if (auto const cpm = dynamic_cast<const Mema::ControlParametersMessage*>(&message))
	{
		DBG(juce::String(__FUNCTION__) << " i:" << cpm->getInputMuteStates().size() << " o:" << cpm->getOutputMuteStates().size() << " c:" << cpm->getCrosspointStates().size());
		for (auto const& inputMuteState : cpm->getInputMuteStates())
			setInputMuteState(inputMuteState.first, inputMuteState.second);
		for (auto const& outputMuteState : cpm->getOutputMuteStates())
			setOutputMuteState(outputMuteState.first, outputMuteState.second);
		for (auto const& crosspointStateKV : cpm->getCrosspointStates())
		{
			auto& inputNumber = crosspointStateKV.first;
			for (auto const& crosspointOStateKV : crosspointStateKV.second)
			{
				auto& outputNumber = crosspointOStateKV.first;
				setMatrixCrosspointEnabledValue(inputNumber, outputNumber, crosspointOStateKV.second.first);
				setMatrixCrosspointFactorValue(inputNumber, outputNumber, crosspointOStateKV.second.second);
			}
		}

		tId = cpm->getType();
	}
	else if (auto const dtsm = dynamic_cast<const Mema::DataTrafficTypeSelectionMessage*>(&message))
	{
		if (!dtsm->hasUserId())
			DBG("Incoming DataTrafficTypeSelecitonMessage cannot be associated with a connection");
		else
			setTrafficTypesForConnectionId(dtsm->getTrafficTypes(), dtsm->getId());

		tId = dtsm->getType();
	}

	std::vector<int> sendIds;
	for (auto const& cId : m_trafficTypesPerConnection)
	{
		// if the connection did not define relevant traffic types, add its connectionId to list of recipients
		if (cId.second.empty())
			sendIds.push_back(cId.first);
		// if the traffic type is active for a connection, add the connectionId to list of recipients
		else if (cId.second.end() != std::find(cId.second.begin(), cId.second.end(), tId))
			sendIds.push_back(cId.first);
	}
	if (!sendIds.empty())
		sendMessageToClients(serializedMessageMemoryBlock, sendIds);
}

void MemaProcessor::sendMessageToClients(const MemoryBlock& messageMemoryBlock, const std::vector<int>& sendIds)
{
	if (m_networkServer && m_networkServer->hasActiveConnections())
	{
		if (!messageMemoryBlock.isEmpty() && !m_networkServer->enqueueMessage(messageMemoryBlock, sendIds))
		{
			auto deadConnectionIds = m_networkServer->cleanupDeadConnections();
			if (!deadConnectionIds.empty())
			{
				for (auto const& dcId : deadConnectionIds)
					m_trafficTypesPerConnection.erase(dcId);
			}
		}
	}
}

double MemaProcessor::getTailLengthSeconds() const
{
	/*dbg*/return 0.0;
}

bool MemaProcessor::acceptsMidi() const
{
	return false;
}

bool MemaProcessor::producesMidi() const
{
	return false;
}

AudioProcessorEditor* MemaProcessor::createEditor()
{
	if (!m_processorEditor)
		m_processorEditor = std::make_unique<MemaProcessorEditor>(this);

	m_processorEditor->onResetToUnity = [=]() { initializeCtrlValuesToUnity(m_inputChannelCount, m_outputChannelCount); };

	return m_processorEditor.get();
}

bool MemaProcessor::hasEditor() const
{
	return !!m_processorEditor;
}

int MemaProcessor::getNumPrograms()
{
	/*dbg*/return 0;
}

int MemaProcessor::getCurrentProgram()
{
	/*dbg*/return 0;
}

void MemaProcessor::setCurrentProgram(int index)
{
	/*dbg*/ignoreUnused(index);
}

const String MemaProcessor::getProgramName(int index)
{
	/*dbg*/ignoreUnused(index);
	/*dbg*/return String();
}

void MemaProcessor::changeProgramName(int index, const String& newName)
{
	/*dbg*/ignoreUnused(index);
	/*dbg*/ignoreUnused(newName);
}

void MemaProcessor::getStateInformation(juce::MemoryBlock& destData)
{
	/*dbg*/ignoreUnused(destData);
}

void MemaProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	/*dbg*/ignoreUnused(data);
	/*dbg*/ignoreUnused(sizeInBytes);
}

void MemaProcessor::audioDeviceIOCallbackWithContext(const float* const* inputChannelData, int numInputChannels,
	float* const* outputChannelData, int numOutputChannels, int numSamples, const AudioIODeviceCallbackContext& context)
{
    ignoreUnused(context);
    
	const juce::ScopedLock sl(m_audioDeviceIOCallbackLock);

	if (m_inputChannelCount != numInputChannels || m_outputChannelCount != numOutputChannels)
	{
		m_inputChannelCount = numInputChannels;
		m_outputChannelCount = numOutputChannels;
		postMessage(std::make_unique<ReinitIOCountMessage>(m_inputChannelCount, m_outputChannelCount).release());
	}

	auto maxActiveChannels = std::max(numInputChannels, numOutputChannels);

	if (s_maxChannelCount < maxActiveChannels)
	{
		jassertfalse;
		return;
	}

	// copy incoming data to processing data buffer
	for (auto i = 0; i < numInputChannels && i < maxActiveChannels; i++)
	{
		memcpy(m_processorChannels[i], inputChannelData[i], (size_t)numSamples * sizeof(float));
	}

	// from juce doxygen: buffer must be the size of max(inCh, outCh) and feeds the input data into the method and is returned with output data
	juce::AudioBuffer<float> audioBufferToProcess(m_processorChannels, maxActiveChannels, numSamples);
    juce::MidiBuffer midiBufferToProcess;
	processBlock(audioBufferToProcess, midiBufferToProcess);

	// copy the processed data buffer data to outgoing data
	auto processedChannelCount = audioBufferToProcess.getNumChannels();
	auto processedSampleCount = audioBufferToProcess.getNumSamples();
	auto processedData = audioBufferToProcess.getArrayOfReadPointers();
	jassert(processedSampleCount == numSamples);
	for (auto i = 0; i < numOutputChannels && i < processedChannelCount; i++)
	{
		memcpy(outputChannelData[i], processedData[i], (size_t)processedSampleCount * sizeof(float));
	}

}

void MemaProcessor::audioDeviceAboutToStart(AudioIODevice* device)
{
	if (device)
    {
        auto activeInputs = device->getActiveInputChannels();
        auto inputChannelCnt  = std::uint16_t(activeInputs.getHighestBit() + 1); // from JUCE documentation
        auto activeOutputs = device->getActiveOutputChannels();
        auto outputChannelCnt = std::uint16_t(activeOutputs.getHighestBit() + 1); // from JUCE documentation
        auto sampleRate = device->getCurrentSampleRate();
        auto bufferSize = device->getCurrentBufferSizeSamples();
        //auto bitDepth = device->getCurrentBitDepth();
        
		setPlayConfigDetails(inputChannelCnt, outputChannelCnt, sampleRate, bufferSize); // redundant to what happens in prepareToPlay to some extent...harmful?
        setChannelCounts(inputChannelCnt, outputChannelCnt);
        prepareToPlay(sampleRate, bufferSize);
    }
}

void MemaProcessor::audioDeviceStopped()
{
	releaseResources();
}

void MemaProcessor::changeListenerCallback(ChangeBroadcaster* source)
{
	if (source == m_deviceManager.get())
		setTimedConfigurationDumpPending();
}

void MemaProcessor::initializeCtrlValues(int inputCount, int outputCount)
{
	std::map<std::uint16_t, bool> inputMuteStates;
	std::map<std::uint16_t, bool> outputMuteStates;
	std::map<std::uint16_t, std::map<std::uint16_t, std::pair<bool, float>>> matrixCrosspointValues;
	{
		// copy the processing relevant variables to not block audio thread during all the xml handling
		const ScopedLock sl(m_audioDeviceIOCallbackLock);
		inputMuteStates = m_inputMuteStates;
		outputMuteStates = m_outputMuteStates;
		matrixCrosspointValues = m_matrixCrosspointValues;
	}

	auto inputChannelCount = std::uint16_t((inputCount > s_minInputsCount) ? inputCount : s_minInputsCount);
	for (std::uint16_t channel = 1; channel <= inputChannelCount; channel++)
	{
		for (auto& inputCommander : m_inputCommanders)
		{
			if (inputCommander != reinterpret_cast<MemaInputCommander*>(&inputCommander))
				inputCommander->setInputMute(channel, inputMuteStates[channel]);
		}
	}

	auto outputChannelCount = std::uint16_t((outputCount > s_minOutputsCount) ? outputCount : s_minOutputsCount);
	for (std::uint16_t channel = 1; channel <= outputChannelCount; channel++)
	{
		for (auto& outputCommander : m_outputCommanders)
		{
			if (outputCommander != reinterpret_cast<MemaOutputCommander*>(&outputCommander))
				outputCommander->setOutputMute(channel, outputMuteStates[channel]);
		}
	}

	for (std::uint16_t in = 1; in <= inputChannelCount; in++)
	{
		for (std::uint16_t out = 1; out <= outputChannelCount; out++)
		{
			for (auto& crosspointCommander : m_crosspointCommanders)
			{
				if (crosspointCommander != reinterpret_cast<MemaCrosspointCommander*>(&crosspointCommander))
				{
					crosspointCommander->setCrosspointEnabledValue(in, out, matrixCrosspointValues[in][out].first);
					crosspointCommander->setCrosspointFactorValue(in, out, matrixCrosspointValues[in][out].second);
				}
			}
		}
	}
}

void MemaProcessor::initializeCtrlValuesToUnity(int inputCount, int outputCount)
{
	auto inputChannelCount = (inputCount > s_minInputsCount) ? inputCount : s_minInputsCount;
	for (std::uint16_t channel = 1; channel <= inputChannelCount; channel++)
		setInputMuteState(channel, false);
    
    auto outputChannelCount = (outputCount > s_minOutputsCount) ? outputCount : s_minOutputsCount;
    for (std::uint16_t channel = 1; channel <= outputChannelCount; channel++)
        setOutputMuteState(channel, false);

    for (std::uint16_t in = 1; in <= inputChannelCount; in++)
	{
		for (std::uint16_t out = 1; out <= outputChannelCount; out++)
		{
			setMatrixCrosspointEnabledValue(in, out, in == out);
			setMatrixCrosspointFactorValue(in, out, 1.0f);
		}
	}
}

void MemaProcessor::setTrafficTypesForConnectionId(const std::vector<SerializableMessage::SerializableMessageType>& trafficTypes, int connectionId)
{
	DBG(juce::String(__FUNCTION__) << " " << connectionId << " chose " << trafficTypes.size() << " types");
	for (auto const& tt : trafficTypes)
	{
		if (m_trafficTypesPerConnection[connectionId].end() == std::find(m_trafficTypesPerConnection[connectionId].begin(), m_trafficTypesPerConnection[connectionId].end(), tt))
			m_trafficTypesPerConnection[connectionId].push_back(tt);
	}
}


} // namespace Mema

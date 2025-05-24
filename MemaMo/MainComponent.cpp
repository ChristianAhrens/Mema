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

#include "MainComponent.h"

#include "CustomPopupMenuComponent.h"
#include "MemaMoComponent.h"
#include "MemaDiscoverComponent.h"
#include "MemaConnectingComponent.h"

#include <AboutComponent.h>
#include <CustomLookAndFeel.h>
#include <WebUpdateDetector.h>

#include <MemaProcessor/MemaMessages.h>
#include <MemaProcessor/MemaServiceData.h>

#include <iOS_utils.h>


MainComponent::MainComponent()
    : juce::Component()
{
    // create the configuration object (is being initialized from disk automatically)
    m_config = std::make_unique<MemaMoAppConfiguration>(JUCEAppBasics::AppConfigurationBase::getDefaultConfigFilePath());
    m_config->addDumper(this);

    // check if config creation was able to read a valid config from disk...
    if (!m_config->isValid())
    {
        m_config->ResetToDefault();
    }

    m_networkConnection = std::make_unique<InterprocessConnectionImpl>();
    m_networkConnection->onConnectionMade = [=]() {
        DBG(__FUNCTION__);

        setStatus(Status::Monitoring);
    };
    m_networkConnection->onConnectionLost = [=]() {
        DBG(__FUNCTION__);
        
        connectToMema();

        setStatus(Status::Connecting);
    };
    m_networkConnection->onMessageReceived = [=](const juce::MemoryBlock& message) {
        auto knownMessage = Mema::SerializableMessage::initFromMemoryBlock(message);
        if (auto const epm = dynamic_cast<const Mema::EnvironmentParametersMessage*>(knownMessage))
        {
            m_settingsHostLookAndFeelId = epm->getPaletteStyle();
            jassert(m_settingsHostLookAndFeelId >= JUCEAppBasics::CustomLookAndFeel::PS_Dark && m_settingsHostLookAndFeelId <= JUCEAppBasics::CustomLookAndFeel::PS_Light);

            if (onPaletteStyleChange && !m_settingsItems[2].second && !m_settingsItems[3].second) // callback must be set and neither 2 nor 3 setting set (manual dark or light)
            {
                m_settingsItems[1].second = 1; // set ticked for setting 1 (follow host)
                onPaletteStyleChange(m_settingsHostLookAndFeelId, false/*do not follow local style any more if a message was received via net once*/);
            }
        }
        else if (m_monitorComponent && nullptr != knownMessage && Status::Monitoring == m_currentStatus)
        {
            m_monitorComponent->handleMessage(*knownMessage);
        }
        Mema::SerializableMessage::freeMessageData(knownMessage);
    };

    m_monitorComponent = std::make_unique<MemaMoComponent>();
    m_monitorComponent->onExitClick = [=]() {
        if (m_discoverComponent)
            m_discoverComponent->setDiscoveredServices(m_availableServices->getServices());

        setStatus(Status::Discovering);
    };
    addAndMakeVisible(m_monitorComponent.get());

    m_discoverComponent = std::make_unique<MemaDiscoverComponent>();
    m_discoverComponent->onServiceSelected = [=](const juce::NetworkServiceDiscovery::Service& selectedService) {
        m_selectedService = selectedService;

        connectToMema();

        if (m_config)
            m_config->triggerConfigurationDump(false);
    };
    addAndMakeVisible(m_discoverComponent.get());

    m_connectingComponent = std::make_unique<MemaConnectingComponent>();
    addAndMakeVisible(m_connectingComponent.get());

    m_aboutComponent = std::make_unique<AboutComponent>(BinaryData::MemaMoRect_png, BinaryData::MemaMoCanvas_pngSize);
    m_aboutButton = std::make_unique<juce::DrawableButton>("About", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_aboutButton->setTooltip(juce::String("About") + juce::JUCEApplication::getInstance()->getApplicationName());
    m_aboutButton->onClick = [this] {
        juce::PopupMenu aboutMenu;
        aboutMenu.addCustomItem(1, std::make_unique<CustomAboutItem>(m_aboutComponent.get(), juce::Rectangle<int>(250, 250)), nullptr, juce::String("Info about") + juce::JUCEApplication::getInstance()->getApplicationName());
        aboutMenu.showMenuAsync(juce::PopupMenu::Options());
    };
    m_aboutButton->setAlwaysOnTop(true);
    m_aboutButton->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    m_aboutButton->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(m_aboutButton.get());

    // default lookandfeel is follow local, therefor none selected
    m_settingsItems[MemaMoSettingsOption::LookAndFeel_FollowHost] = std::make_pair("Follow Mema", 0);
    m_settingsItems[MemaMoSettingsOption::LookAndFeel_Dark] = std::make_pair("Dark", 1);
    m_settingsItems[MemaMoSettingsOption::LookAndFeel_Light] = std::make_pair("Light", 0);
    // default output visu is normal meterbridge
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_Meterbridge] = std::make_pair("Meterbridge", 1);
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_LRS] = std::make_pair(juce::AudioChannelSet::createLRS().getDescription().toStdString(), 0);
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_LCRS] = std::make_pair(juce::AudioChannelSet::createLCRS().getDescription().toStdString(), 0);
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_5point0] = std::make_pair(juce::AudioChannelSet::create5point0().getDescription().toStdString(), 0);
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_5point1] = std::make_pair(juce::AudioChannelSet::create5point1().getDescription().toStdString(), 0);
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_5point1point2] = std::make_pair(juce::AudioChannelSet::create5point1point2().getDescription().toStdString(), 0);
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_7point0] = std::make_pair(juce::AudioChannelSet::create7point0().getDescription().toStdString(), 0);
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_7point1] = std::make_pair(juce::AudioChannelSet::create7point1().getDescription().toStdString(), 0);
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_7point1point4] = std::make_pair(juce::AudioChannelSet::create7point1point4().getDescription().toStdString(), 0);
    m_settingsItems[MemaMoSettingsOption::OutputVisuType_9point1point6] = std::make_pair(juce::AudioChannelSet::create9point1point6().getDescription().toStdString(), 0);
    // default metering colour is green
    m_settingsItems[MemaMoSettingsOption::MeteringColour_Green] = std::make_pair("Green", 1);
    m_settingsItems[MemaMoSettingsOption::MeteringColour_Red] = std::make_pair("Red", 0);
    m_settingsItems[MemaMoSettingsOption::MeteringColour_Blue] = std::make_pair("Blue", 0);
    m_settingsItems[MemaMoSettingsOption::MeteringColour_Pink] = std::make_pair("Anni Pink", 0);
    m_settingsButton = std::make_unique<juce::DrawableButton>("Settings", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_settingsButton->setTooltip(juce::String("Settings for") + juce::JUCEApplication::getInstance()->getApplicationName());
    m_settingsButton->onClick = [this] {
        juce::PopupMenu lookAndFeelSubMenu;
        for (int i = MemaMoSettingsOption::LookAndFeel_First; i <= MemaMoSettingsOption::LookAndFeel_Last; i++)
            lookAndFeelSubMenu.addItem(i, m_settingsItems[i].first, true, m_settingsItems[i].second == 1);

        juce::PopupMenu outputVisuTypeSubMenu;
        for (int i = MemaMoSettingsOption::OutputVisuType_First; i <= MemaMoSettingsOption::OutputVisuType_Last; i++)
            outputVisuTypeSubMenu.addItem(i, m_settingsItems[i].first, true, m_settingsItems[i].second == 1);

        juce::PopupMenu meteringColourSubMenu;
        for (int i = MemaMoSettingsOption::MeteringColour_First; i <= MemaMoSettingsOption::MeteringColour_Last; i++)
            meteringColourSubMenu.addItem(i, m_settingsItems[i].first, true, m_settingsItems[i].second == 1);

        juce::PopupMenu settingsMenu;
        settingsMenu.addSubMenu("LookAndFeel", lookAndFeelSubMenu);
        settingsMenu.addSubMenu("Output monitoring", outputVisuTypeSubMenu);
        settingsMenu.addSubMenu("Metering colour", meteringColourSubMenu);
        settingsMenu.showMenuAsync(juce::PopupMenu::Options(), [=](int selectedId) {
            handleSettingsMenuResult(selectedId);
            if (m_config)
                m_config->triggerConfigurationDump();
        });
    };
    m_settingsButton->setAlwaysOnTop(true);
    m_settingsButton->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    m_settingsButton->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(m_settingsButton.get());

    m_disconnectButton = std::make_unique<juce::DrawableButton>("Disconnect", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_disconnectButton->setTooltip(juce::String("Disconnect ") + juce::JUCEApplication::getInstance()->getApplicationName());
    m_disconnectButton->onClick = [this] {
        if (m_networkConnection)
            m_networkConnection->disconnect();

        m_selectedService = {};

        if (m_config)
            m_config->triggerConfigurationDump();

        if (m_discoverComponent)
            m_discoverComponent->setDiscoveredServices(m_availableServices->getServices());

        setStatus(Status::Discovering);
    };
    m_disconnectButton->setAlwaysOnTop(true);
    m_disconnectButton->setColour(juce::DrawableButton::ColourIds::backgroundColourId, juce::Colours::transparentBlack);
    m_disconnectButton->setColour(juce::DrawableButton::ColourIds::backgroundOnColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(m_disconnectButton.get());

    m_availableServices = std::make_unique<juce::NetworkServiceDiscovery::AvailableServiceList>(
        Mema::ServiceData::getServiceTypeUID(), 
        Mema::ServiceData::getBroadcastPort());
    m_availableServices->onChange = [=]() { 
        if (m_discoverComponent)
            m_discoverComponent->setDiscoveredServices(m_availableServices->getServices());
    };

#ifdef NIX//DEBUG
    auto inputs = 11;
    auto outputs = 12;
    auto buffer = juce::AudioBuffer<float>();
    auto refSample = 11.11f;
    auto sr = 48000;
    auto mespb = 256;

    auto apm = std::make_unique<Mema::AnalyzerParametersMessage>(sr, mespb);
    auto apmb = apm->getSerializedMessage();
    auto apmcpy = Mema::AnalyzerParametersMessage(apmb);
    auto test5 = apmcpy.getSampleRate();
    auto test6 = apmcpy.getMaximumExpectedSamplesPerBlock();
    jassert(test5 == sr);
    jassert(test6 == mespb);

    auto rcm = std::make_unique<Mema::ReinitIOCountMessage>(inputs, outputs);
    auto rcmb = rcm->getSerializedMessage();
    auto rcmcpy = Mema::ReinitIOCountMessage(rcmb);
    auto test7 = rcmcpy.getInputCount();
    auto test8 = rcmcpy.getOutputCount();
    jassert(test7 == inputs);
    jassert(test8 == outputs);

    auto channelCount = 2;
    auto sampleCount = 6;
    buffer.setSize(channelCount, sampleCount, false, true, false);
    for (int i = 0; i < channelCount; i++)
    {
        for (int j = 0; j < sampleCount; j++)
        {
            buffer.setSample(i, j, ++refSample);
        }
    }
    auto rrefSample1 = refSample;
    auto aibm1 = std::make_unique<Mema::AudioInputBufferMessage>(buffer);
    for (int i = channelCount - 1; i >= 0; i--)
    {
        for (int j = sampleCount - 1; j >= 0; j--)
        {
            auto test1 = aibm1->getAudioBuffer().getSample(i, j);
            jassert(int(test1) == int(refSample));
            refSample--;
        }
    }
    auto aibmb1 = aibm1->getSerializedMessage();
    auto aibmcpy1 = Mema::AudioInputBufferMessage(aibmb1);
    for (int i = channelCount - 1; i >= 0; i--)
    {
        for (int j = sampleCount - 1; j >= 0; j--)
        {
            auto test1 = aibmcpy1.getAudioBuffer().getSample(i, j);
            jassert(int(test1) == int(rrefSample1));
            rrefSample1--;
        }
    }

    buffer.setSize(channelCount, sampleCount, false, true, false);
    for (int i = 0; i < channelCount; i++)
    {
        for (int j = 0; j < sampleCount; j++)
        {
            buffer.setSample(i, j, ++refSample);
        }
    }
    auto rrefSample2 = refSample;
    auto aibm2 = std::make_unique<Mema::AudioOutputBufferMessage>(buffer);
    for (int i = channelCount - 1; i >= 0; i--)
    {
        for (int j = sampleCount - 1; j >= 0; j--)
        {
            auto test2 = aibm2->getAudioBuffer().getSample(i, j);
            jassert(int(test2) == int(rrefSample2));
            rrefSample2--;
        }
    }
    auto aibmb2 = aibm2->getSerializedMessage();
    auto aibmcpy2 = Mema::AudioOutputBufferMessage(aibmb2);
    for (int i = channelCount - 1; i >= 0; i--)
    {
        for (int j = sampleCount - 1; j >= 0; j--)
        {
            auto test2 = aibmcpy2.getAudioBuffer().getSample(i, j);
            jassert(int(test2) == int(refSample));
            refSample--;
        }
    }
#endif

    setSize(400, 350);

#if defined JUCE_IOS
    // iOS is updated via AppStore
#define IGNORE_UPDATES
#elif defined JUCE_ANDROID
    // Android as well
#define IGNORE_UPDATES
#endif

#if defined IGNORE_UPDATES
#else
    auto updater = JUCEAppBasics::WebUpdateDetector::getInstance();
    updater->SetReferenceVersion(ProjectInfo::versionString);
    updater->SetDownloadUpdateWebAddress("https://github.com/christianahrens/mema/releases/latest");
    updater->CheckForNewVersion(true, "https://raw.githubusercontent.com/ChristianAhrens/Mema/refs/heads/main/");
#endif


    // add this main component to watchers
    m_config->addWatcher(this); // without initial update - that we have to do externally after lambdas were assigned

}

MainComponent::~MainComponent()
{
}

void MainComponent::resized()
{
    auto safety = JUCEAppBasics::iOS_utils::getDeviceSafetyMargins();
    auto safeBounds = getLocalBounds();
    safeBounds.removeFromTop(safety._top);
    safeBounds.removeFromBottom(safety._bottom);
    safeBounds.removeFromLeft(safety._left);
    safeBounds.removeFromRight(safety._right);
    
    switch (m_currentStatus)
    {
        case Status::Monitoring:
            m_connectingComponent->setVisible(false);
            m_discoverComponent->setVisible(false);
            m_monitorComponent->setVisible(true);
            m_monitorComponent->setBounds(safeBounds);
            break;
        case Status::Connecting:
            m_monitorComponent->setVisible(false);
            m_discoverComponent->setVisible(false);
            m_connectingComponent->setVisible(true);
            m_connectingComponent->setBounds(safeBounds);
            break;
        case Status::Discovering:
        default:
            m_connectingComponent->setVisible(false);
            m_monitorComponent->setVisible(false);
            m_discoverComponent->setVisible(true);
            m_discoverComponent->setBounds(safeBounds);
            break;
    }

    m_aboutButton->setBounds(safeBounds.removeFromTop(35).removeFromLeft(30).removeFromBottom(30));
    m_settingsButton->setBounds(safeBounds.removeFromTop(35).removeFromLeft(30).removeFromBottom(30));
    m_disconnectButton->setBounds(safeBounds.removeFromTop(35).removeFromLeft(30).removeFromBottom(30));
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::lookAndFeelChanged()
{
    auto aboutButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::question_mark_24dp_svg).get());
    aboutButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_aboutButton->setImages(aboutButtonDrawable.get());

    auto settingsDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::settings_24dp_svg).get());
    settingsDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_settingsButton->setImages(settingsDrawable.get());

    auto disconnectDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::link_off_24dp_svg).get());
    disconnectDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_disconnectButton->setImages(disconnectDrawable.get());

    applyMeteringColour();
}

void MainComponent::applySettingsOption(const MemaMoSettingsOption& option)
{
    // use the settings menu item call infrastructure to set the option
    handleSettingsMenuResult(option);
}

void MainComponent::handleSettingsMenuResult(int selectedId)
{
    if (0 == selectedId)
        return; // nothing selected, dismiss
    else if (MemaMoSettingsOption::LookAndFeel_First <= selectedId && MemaMoSettingsOption::LookAndFeel_Last >= selectedId)
        handleSettingsLookAndFeelMenuResult(selectedId);
    else if (MemaMoSettingsOption::OutputVisuType_First <= selectedId && MemaMoSettingsOption::OutputVisuType_Last >= selectedId)
        handleSettingsOutputVisuTypeMenuResult(selectedId);
    else if (MemaMoSettingsOption::MeteringColour_First <= selectedId && MemaMoSettingsOption::MeteringColour_Last >= selectedId)
        handleSettingsMeteringColourMenuResult(selectedId);
    else
        jassertfalse; // unhandled menu entry!?
}

void MainComponent::handleSettingsLookAndFeelMenuResult(int selectedId)
{
    // helper internal function to avoid code clones
    std::function<void(int, int, int)> setSettingsItemsCheckState = [=](int a, int b, int c) {
        m_settingsItems[MemaMoSettingsOption::LookAndFeel_FollowHost].second = a;
        m_settingsItems[MemaMoSettingsOption::LookAndFeel_Dark].second = b;
        m_settingsItems[MemaMoSettingsOption::LookAndFeel_Light].second = c;
    };

    switch (selectedId)
    {
    case MemaMoSettingsOption::LookAndFeel_FollowHost:
        setSettingsItemsCheckState(1, 0, 0);
        if (onPaletteStyleChange && m_settingsHostLookAndFeelId != -1)
            onPaletteStyleChange(m_settingsHostLookAndFeelId, false);
        break;
    case MemaMoSettingsOption::LookAndFeel_Dark:
        setSettingsItemsCheckState(0, 1, 0);
        if (onPaletteStyleChange)
            onPaletteStyleChange(JUCEAppBasics::CustomLookAndFeel::PS_Dark, false);
        break;
    case MemaMoSettingsOption::LookAndFeel_Light:
        setSettingsItemsCheckState(0, 0, 1);
        if (onPaletteStyleChange)
            onPaletteStyleChange(JUCEAppBasics::CustomLookAndFeel::PS_Light, false);
        break;
    default:
        jassertfalse; // unknown id fed in unintentionally ?!
        break;
    }
}

void MainComponent::handleSettingsOutputVisuTypeMenuResult(int selectedId)
{
    // helper internal function to avoid code clones
    std::function<void(int, int, int, int, int, int, int, int, int, int)> setSettingsItemsCheckState = [=](int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_Meterbridge].second = a;
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_LRS].second = b;
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_LCRS].second = c;
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_5point0].second = d;
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_5point1].second = e;
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_5point1point2].second = f;
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_7point0].second = g;
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_7point1].second = h;
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_7point1point4].second = i;
        m_settingsItems[MemaMoSettingsOption::OutputVisuType_9point1point6].second = j;
    };

    switch (selectedId)
    {
    case MemaMoSettingsOption::OutputVisuType_Meterbridge:
        setSettingsItemsCheckState(1, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        if (m_monitorComponent)
            m_monitorComponent->setOutputMeteringVisuActive();
        break;
    case MemaMoSettingsOption::OutputVisuType_LRS:
        setSettingsItemsCheckState(0, 1, 0, 0, 0, 0, 0, 0, 0, 0);
        if (m_monitorComponent)
            m_monitorComponent->setOutputFieldVisuActive(juce::AudioChannelSet::createLRS());
        break;
    case MemaMoSettingsOption::OutputVisuType_LCRS:
        setSettingsItemsCheckState(0, 0, 1, 0, 0, 0, 0, 0, 0, 0);
        if (m_monitorComponent)
            m_monitorComponent->setOutputFieldVisuActive(juce::AudioChannelSet::createLCRS());
        break;
    case MemaMoSettingsOption::OutputVisuType_5point0:
        setSettingsItemsCheckState(0, 0, 0, 1, 0, 0, 0, 0, 0, 0);
        if (m_monitorComponent)
            m_monitorComponent->setOutputFieldVisuActive(juce::AudioChannelSet::create5point0());
        break;
    case MemaMoSettingsOption::OutputVisuType_5point1:
        setSettingsItemsCheckState(0, 0, 0, 0, 1, 0, 0, 0, 0, 0);
        if (m_monitorComponent)
            m_monitorComponent->setOutputFieldVisuActive(juce::AudioChannelSet::create5point1());
        break;
    case MemaMoSettingsOption::OutputVisuType_5point1point2:
        setSettingsItemsCheckState(0, 0, 0, 0, 0, 1, 0, 0, 0, 0);
        if (m_monitorComponent)
            m_monitorComponent->setOutputFieldVisuActive(juce::AudioChannelSet::create5point1point2());
        break;
    case MemaMoSettingsOption::OutputVisuType_7point0:
        setSettingsItemsCheckState(0, 0, 0, 0, 0, 0, 1, 0, 0, 0);
        if (m_monitorComponent)
            m_monitorComponent->setOutputFieldVisuActive(juce::AudioChannelSet::create7point0());
        break;
    case MemaMoSettingsOption::OutputVisuType_7point1:
        setSettingsItemsCheckState(0, 0, 0, 0, 0, 0, 0, 1, 0, 0);
        if (m_monitorComponent)
            m_monitorComponent->setOutputFieldVisuActive(juce::AudioChannelSet::create7point1());
        break;
    case MemaMoSettingsOption::OutputVisuType_7point1point4:
        setSettingsItemsCheckState(0, 0, 0, 0, 0, 0, 0, 0, 1, 0);
        if (m_monitorComponent)
            m_monitorComponent->setOutputFieldVisuActive(juce::AudioChannelSet::create7point1point4());
        break;
    case MemaMoSettingsOption::OutputVisuType_9point1point6:
        setSettingsItemsCheckState(0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
        if (m_monitorComponent)
            m_monitorComponent->setOutputFieldVisuActive(juce::AudioChannelSet::create9point1point6());
        break;
    default:
        jassertfalse; // unknown id fed in unintentionally ?!
        break;
    }

    resized();
}

void MainComponent::handleSettingsMeteringColourMenuResult(int selectedId)
{
    // helper internal function to avoid code clones
    std::function<void(int, int, int, int)> setSettingsItemsCheckState = [=](int green, int red, int blue, int pink) {
        m_settingsItems[MemaMoSettingsOption::MeteringColour_Green].second = green;
        m_settingsItems[MemaMoSettingsOption::MeteringColour_Red].second = red;
        m_settingsItems[MemaMoSettingsOption::MeteringColour_Blue].second = blue;
        m_settingsItems[MemaMoSettingsOption::MeteringColour_Pink].second = pink;
    };

    switch (selectedId)
    {
    case MemaMoSettingsOption::MeteringColour_Green:
        setSettingsItemsCheckState(1, 0, 0, 0);
        setMeteringColour(juce::Colours::forestgreen);
        break;
    case MemaMoSettingsOption::MeteringColour_Red:
        setSettingsItemsCheckState(0, 1, 0, 0);
        setMeteringColour(juce::Colours::orangered);
        break;
    case MemaMoSettingsOption::MeteringColour_Blue:
        setSettingsItemsCheckState(0, 0, 1, 0);
        setMeteringColour(juce::Colours::dodgerblue);
        break;
    case MemaMoSettingsOption::MeteringColour_Pink:
        setSettingsItemsCheckState(0, 0, 0, 1);
        setMeteringColour(juce::Colours::deeppink);
        break;
    default:
        break;
    }
}

void MainComponent::setMeteringColour(const juce::Colour& meteringColour)
{
    m_meteringColour = meteringColour;

    applyMeteringColour();
}

void MainComponent::applyMeteringColour()
{
    auto customLookAndFeel = dynamic_cast<JUCEAppBasics::CustomLookAndFeel*>(&getLookAndFeel());
    if (customLookAndFeel)
    {
        switch (customLookAndFeel->getPaletteStyle())
        {
        case JUCEAppBasics::CustomLookAndFeel::PS_Light:
            getLookAndFeel().setColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringPeakColourId, m_meteringColour.brighter());
            getLookAndFeel().setColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId, m_meteringColour);
            break;
        case JUCEAppBasics::CustomLookAndFeel::PS_Dark:
        default:
            getLookAndFeel().setColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringPeakColourId, m_meteringColour.darker());
            getLookAndFeel().setColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId, m_meteringColour);
            break;
        }
    }
}

void MainComponent::setStatus(const Status& s)
{
    m_currentStatus = s;
    resized();
}

const MainComponent::Status MainComponent::getStatus()
{
    return m_currentStatus;
}

void MainComponent::connectToMema()
{
    setStatus(Status::Connecting);

    timerCallback(); // avoid codeclones by manually trigger the timed connection attempt once

    // restart connection attempt after 5s, in case something got stuck...
    startTimer(5000);
}

void MainComponent::timerCallback()
{
    if (Status::Connecting == getStatus())
    {
        auto sl = m_availableServices->getServices();
        auto const& iter = std::find_if(sl.begin(), sl.end(), [=](const auto& service) { return service.description == m_selectedService.description; });
        if (iter != sl.end())
        {
            if ((m_selectedService.address != iter->address && m_selectedService.port != iter->port && m_selectedService.description != iter->description) || !m_networkConnection->isConnected())
            {
                m_selectedService = *iter;
                if (m_networkConnection)
                    m_networkConnection->ConnectToSocket(m_selectedService.address.toString(), m_selectedService.port);
            }
            else if (m_networkConnection && !m_networkConnection->isConnected())
                m_networkConnection->RetryConnectToSocket();
        }
    }
    else
        stopTimer();
}

void MainComponent::performConfigurationDump()
{
    if (m_config)
    {
        // connection config
        auto connectionConfigXmlElement = std::make_unique<juce::XmlElement>(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::CONNECTIONCONFIG));

        auto serviceDescriptionXmlElmement = std::make_unique<juce::XmlElement>(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::SERVICEDESCRIPTION));
        serviceDescriptionXmlElmement->addTextElement(m_selectedService.description);
        connectionConfigXmlElement->addChildElement(serviceDescriptionXmlElmement.release());

        m_config->setConfigState(std::move(connectionConfigXmlElement), MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::CONNECTIONCONFIG));

        // visu config
        auto visuConfigXmlElement = std::make_unique<juce::XmlElement>(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::VISUCONFIG));
        
        auto lookAndFeelXmlElmement = std::make_unique<juce::XmlElement>(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::LOOKANDFEEL));
        for (int i = MemaMoSettingsOption::LookAndFeel_First; i <= MemaMoSettingsOption::LookAndFeel_Last; i++)
        {
            if (m_settingsItems[i].second == 1)
                lookAndFeelXmlElmement->addTextElement(juce::String(i));
        }
        visuConfigXmlElement->addChildElement(lookAndFeelXmlElmement.release());

        auto outputVisuTypeXmlElmement = std::make_unique<juce::XmlElement>(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::OUTPUTVISUTYPE));
        for (int i = MemaMoSettingsOption::OutputVisuType_First; i <= MemaMoSettingsOption::OutputVisuType_Last; i++)
        {
            if (m_settingsItems[i].second == 1)
                outputVisuTypeXmlElmement->addTextElement(juce::String(i));
        }
        visuConfigXmlElement->addChildElement(outputVisuTypeXmlElmement.release());
        
        auto meteringColourXmlElmement = std::make_unique<juce::XmlElement>(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::METERINGCOLOUR));
        for (int i = MemaMoSettingsOption::MeteringColour_First; i <= MemaMoSettingsOption::MeteringColour_Last; i++)
        {
            if (m_settingsItems[i].second == 1)
                meteringColourXmlElmement->addTextElement(juce::String(i));
        }
        visuConfigXmlElement->addChildElement(meteringColourXmlElmement.release());

        m_config->setConfigState(std::move(visuConfigXmlElement), MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::VISUCONFIG));
    }
}

void MainComponent::onConfigUpdated()
{
    auto connectionConfigState = m_config->getConfigState(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::CONNECTIONCONFIG));
    if (connectionConfigState)
    {
        auto serviceDescriptionXmlElement = connectionConfigState->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::SERVICEDESCRIPTION));
        if (serviceDescriptionXmlElement)
        {
            auto serviceDescription = serviceDescriptionXmlElement->getAllSubText();
            if (serviceDescription.isNotEmpty() && m_selectedService.description != serviceDescription)
            {
                if (m_networkConnection)
                    m_networkConnection->disconnect();

                m_selectedService = {};
                m_selectedService.description = serviceDescription;

                connectToMema();
            }
        }
    }

    auto visuConfigState = m_config->getConfigState(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::VISUCONFIG));
    if (visuConfigState)
    {
        auto lookAndFeelXmlElement = visuConfigState->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::LOOKANDFEEL));
        if (lookAndFeelXmlElement)
        {
            auto lookAndFeelSettingsOptionId = lookAndFeelXmlElement->getAllSubText().getIntValue();
            handleSettingsLookAndFeelMenuResult(lookAndFeelSettingsOptionId);
        }

        auto outputVisuTypeXmlElement = visuConfigState->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::OUTPUTVISUTYPE));
        if (outputVisuTypeXmlElement)
        {
            auto outputVisuTypeSettingsOptionId = outputVisuTypeXmlElement->getAllSubText().getIntValue();
            handleSettingsOutputVisuTypeMenuResult(outputVisuTypeSettingsOptionId);
        }

        auto meteringColourXmlElement = visuConfigState->getChildByName(MemaMoAppConfiguration::getTagName(MemaMoAppConfiguration::TagID::METERINGCOLOUR));
        if (meteringColourXmlElement)
        {
            auto meteringColourSettingsOptionId = meteringColourXmlElement->getAllSubText().getIntValue();
            handleSettingsMeteringColourMenuResult(meteringColourSettingsOptionId);
        }
    }
}


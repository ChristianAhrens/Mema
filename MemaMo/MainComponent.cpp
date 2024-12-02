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

#include "MainComponent.h"

#include "CustomPopupMenuComponent.h"
#include "MemaMoComponent.h"
#include "MemaDiscoverComponent.h"

#include <AboutComponent.h>
#include <CustomLookAndFeel.h>
#include <WebUpdateDetector.h>

#include <MemaProcessor/MemaMessages.h>
#include <MemaProcessor/MemaServiceData.h>

#include <iOS_utils.h>


MainComponent::MainComponent()
    : juce::Component()
{
    m_networkConnection = std::make_unique<InterprocessConnectionImpl>();
    m_networkConnection->onConnectionMade = [=]() {
        DBG(__FUNCTION__);
    };
    m_networkConnection->onConnectionLost = [=]() {
        DBG(__FUNCTION__);
        if (m_monitorComponent)
            m_monitorComponent->setRunning(false);

        if (m_discoverComponent)
            m_discoverComponent->setDiscoveredServices(m_availableServices->getServices());

        m_currentStatus = Status::Discovering;
        resized();
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
        else if (m_monitorComponent && nullptr != knownMessage)
            m_monitorComponent->handleMessage(*knownMessage);
        Mema::SerializableMessage::freeMessageData(knownMessage);
    };

    m_monitorComponent = std::make_unique<MemaMoComponent>();
    m_monitorComponent->onExitClick = [=]() {
        if (m_discoverComponent)
            m_discoverComponent->setDiscoveredServices(m_availableServices->getServices());

        if (m_monitorComponent)
            m_monitorComponent->setRunning(false);

        m_currentStatus = Status::Discovering;
        resized();
    };
    m_monitorComponent->setRunning(false);
    addAndMakeVisible(m_monitorComponent.get());

    m_discoverComponent = std::make_unique<MemaDiscoverComponent>();
    m_discoverComponent->onServiceSelected = [=](const juce::NetworkServiceDiscovery::Service& selectedService) {
        m_currentStatus = Status::Monitoring;
        resized();

        if (m_monitorComponent)
            m_monitorComponent->setRunning(true);

        if (m_networkConnection)
            m_networkConnection->connectToSocket(selectedService.address.toString(), selectedService.port, 100);
    };
    addAndMakeVisible(m_discoverComponent.get());

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

    //default lookandfeel is follow local, therefor none selected
    m_settingsItems[1] = std::make_pair("Follow host", 0);
    m_settingsItems[2] = std::make_pair("Dark", 0);
    m_settingsItems[3] = std::make_pair("Light", 0);
    m_settingsButton = std::make_unique<juce::DrawableButton>("Settings", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_settingsButton->setTooltip(juce::String("Settings for") + juce::JUCEApplication::getInstance()->getApplicationName());
    m_settingsButton->onClick = [this] {
        juce::PopupMenu settingsMenu;
        settingsMenu.addSectionHeader("LookAndFeel");
        
        for(auto const& item : m_settingsItems)
        {
            juce::PopupMenu::Item followHostItem("Follow host");
            followHostItem.setTicked();
            settingsMenu.addItem(item.first, item.second.first, true, item.second.second == 1);
        }

        settingsMenu.showMenuAsync(juce::PopupMenu::Options(), [=](int selectedId) {
            switch (selectedId)
            {
            case 1:
                m_settingsItems[1].second = 1;
                m_settingsItems[2].second = 0;
                m_settingsItems[3].second = 0;
                if (onPaletteStyleChange && m_settingsHostLookAndFeelId != -1)
                    onPaletteStyleChange(m_settingsHostLookAndFeelId, false);
                break;
            case 2:
                m_settingsItems[1].second = 0;
                m_settingsItems[2].second = 1;
                m_settingsItems[3].second = 0;
                if (onPaletteStyleChange)
                    onPaletteStyleChange(JUCEAppBasics::CustomLookAndFeel::PS_Dark, false);
                break;
            case 3:
                m_settingsItems[1].second = 0;
                m_settingsItems[2].second = 0;
                m_settingsItems[3].second = 1;
                if (onPaletteStyleChange)
                    onPaletteStyleChange(JUCEAppBasics::CustomLookAndFeel::PS_Light, false);
                break;
            default:
                break;
            }
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

        if (m_monitorComponent)
            m_monitorComponent->setRunning(false);

        if (m_discoverComponent)
            m_discoverComponent->setDiscoveredServices(m_availableServices->getServices());

        m_currentStatus = Status::Discovering;
        resized();
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
            m_discoverComponent->setVisible(false);
            m_monitorComponent->setVisible(true);
            m_monitorComponent->setBounds(safeBounds);
            break;
        case Status::Discovering:
        default:
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
}


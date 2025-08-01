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

#include "MemaUIComponent.h"

#include "AboutComponent.h"
#include "CustomPopupMenuComponent.h"

#include <AppConfigurationBase.h>

//==============================================================================
class LoadBar : public juce::Component
{
public:
    LoadBar(juce::String label, bool showPercent = true, bool showMax = true) : juce::Component::Component() { m_label = label; m_showPercent = showPercent; m_showMax = showMax; }
    ~LoadBar() {}

    //==============================================================================
    void paint(Graphics& g) override
    {
        auto margin = 1.0f;

        auto bounds = getLocalBounds().toFloat();

        g.setColour(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::windowBackground));
        g.fillRect(bounds);

        auto barBounds = bounds.reduced(margin);

        auto loadBarHeight = barBounds.getHeight();
        if (!m_loadsPercent.empty())
            loadBarHeight = (barBounds.getHeight() / m_loadsPercent.size());
        
        auto avgLoad = 0;
        auto maxLoad = 0;
        int i = 0;
        for (auto & loadPercentKV : m_loadsPercent)
        {
            auto loadPercent = loadPercentKV.second;
            auto alert = m_alerts[loadPercentKV.first];
            
            auto normalPercent = loadPercent;
            auto warningPercent = 0;
            auto criticalPercent = 0;
            if (loadPercent > 75)
            {
                normalPercent = 75;
                warningPercent = loadPercent - normalPercent;
            }
            if (loadPercent > 95)
            {
                warningPercent = 20;
                criticalPercent = loadPercent - normalPercent - warningPercent;
            }
            if (loadPercent >= 100)
            {
                criticalPercent = 5;
            }

            auto individualBarBounds = barBounds.removeFromTop(loadBarHeight);
            if (i < m_loadsPercent.size()-1)
                individualBarBounds.removeFromBottom(margin);

            if (alert)
            {
                g.setColour(juce::Colour(0xff, 0x30, 0x02).withAlpha(0.8f));
                g.fillRect(individualBarBounds);
            }
            else
            {
                g.setColour(getLookAndFeel().findColour(juce::TextButton::ColourIds::buttonColourId));
                g.fillRect(individualBarBounds.removeFromLeft(individualBarBounds.getWidth() * (float(normalPercent) / 100.0f)));
                if (warningPercent > 0)
                {
                    g.setColour(juce::Colour(0xff, 0xe8, 0x00).withAlpha(0.5f));
                    g.fillRect(individualBarBounds.removeFromLeft(individualBarBounds.getWidth() * (float(warningPercent) / 25.0f)));
                }
                if (criticalPercent > 0)
                {
                    g.setColour(juce::Colour(0xff, 0x40, 0x02).withAlpha(0.5f));
                    g.fillRect(individualBarBounds.removeFromLeft(individualBarBounds.getWidth() * (float(criticalPercent) / 5.0f)));
                }
            }

            avgLoad += loadPercent;
            if (maxLoad < loadPercent)
                maxLoad = loadPercent;
            i++;
        }

        if (!m_loadsPercent.empty())
            avgLoad /= int(m_loadsPercent.size());

        auto labelText = m_label;
        if (m_showPercent)
            labelText += juce::String(" ") + juce::String(m_showMax ? maxLoad : avgLoad) + juce::String("%");

        g.setColour(getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
        g.drawText(labelText, bounds, juce::Justification::centred);
    };

    //==============================================================================
    void setLoadPercent(int loadPercent, int id = 0)
    {
        m_loadsPercent[id] = loadPercent;
        repaint();
    };
    void setAlert(bool alert, int id = 0)
    {
        m_alerts[id] = alert;
        repaint();
    };

private:
    std::map<int, int> m_loadsPercent;
    std::map<int, bool> m_alerts;
    juce::String m_label;
    bool m_showPercent = false;
    bool m_showMax = false;
};

//==============================================================================
class EmptySpace :public juce::Component
{
public:
    EmptySpace() : juce::Component::Component() {}
    ~EmptySpace() {}

    //==============================================================================
    void paint(Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    };
};

namespace Mema
{

MemaUIComponent::MemaUIComponent()
    : juce::Component()
{
    setOpaque(true);

    m_toggleStandaloneWindowButton = std::make_unique<juce::DrawableButton>("Show as standalone window", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_toggleStandaloneWindowButton->setTooltip("Show as standalone window");
    m_toggleStandaloneWindowButton->onClick = [this] { if (onToggleStandaloneWindow) onToggleStandaloneWindow(!m_isStandaloneWindow); };
#if JUCE_LINUX
    m_toggleStandaloneWindowButton->setEnabled(false);
#endif
    addAndMakeVisible(m_toggleStandaloneWindowButton.get());

    m_audioSettingsButton = std::make_unique<juce::DrawableButton>("Audio Device Setup", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_audioSettingsButton->setTooltip("Audio Device Setup");
    m_audioSettingsButton->onClick = [this] {
        if (onSetupMenuClicked) onSetupMenuClicked();
    };
    addAndMakeVisible(m_audioSettingsButton.get());

    // default lookandfeel is follow local, therefor none selected
    m_settingsItems[MemaSettingsOption::LookAndFeel_Automatic] = std::make_pair("Automatic", 1);
    m_settingsItems[MemaSettingsOption::LookAndFeel_Dark] = std::make_pair("Dark", 0);
    m_settingsItems[MemaSettingsOption::LookAndFeel_Light] = std::make_pair("Light", 0);
    // default metering colour is green
    m_settingsItems[MemaSettingsOption::MeteringColour_Green] = std::make_pair("Green", 1);
    m_settingsItems[MemaSettingsOption::MeteringColour_Red] = std::make_pair("Red", 0);
    m_settingsItems[MemaSettingsOption::MeteringColour_Blue] = std::make_pair("Blue", 0);
    m_settingsItems[MemaSettingsOption::MeteringColour_Pink] = std::make_pair("Anni Pink", 0);
    m_appSettingsButton = std::make_unique<juce::DrawableButton>("Application settings", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_appSettingsButton->setTooltip("Application settings");
    m_appSettingsButton->onClick = [this] {
        juce::PopupMenu lookAndFeelSubMenu;
        for (int i = MemaSettingsOption::LookAndFeel_First; i <= MemaSettingsOption::LookAndFeel_Last; i++)
            lookAndFeelSubMenu.addItem(i, m_settingsItems[i].first, true, m_settingsItems[i].second == 1);

        juce::PopupMenu meteringColourSubMenu;
        for (int i = MemaSettingsOption::MeteringColour_First; i <= MemaSettingsOption::MeteringColour_Last; i++)
            meteringColourSubMenu.addItem(i, m_settingsItems[i].first, true, m_settingsItems[i].second == 1);

        juce::PopupMenu settingsMenu;
        settingsMenu.addSubMenu("LookAndFeel", lookAndFeelSubMenu);
        settingsMenu.addSubMenu("Metering colour", meteringColourSubMenu);
        settingsMenu.showMenuAsync(juce::PopupMenu::Options(), [=](int selectedId) { handleSettingsMenuResult(selectedId); });
    };
    addAndMakeVisible(m_appSettingsButton.get());

    m_aboutComponent = std::make_unique<AboutComponent>(BinaryData::MemaRect_png, BinaryData::MemaRect_pngSize);
    m_aboutButton = std::make_unique<juce::DrawableButton>("About", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_aboutButton->setTooltip(juce::String("About") + juce::JUCEApplication::getInstance()->getApplicationName());
    m_aboutButton->onClick = [this] {
        juce::PopupMenu aboutMenu;
        aboutMenu.addCustomItem(1, std::make_unique<CustomAboutItem>(m_aboutComponent.get(), juce::Rectangle<int>(250, 250)), nullptr, juce::String("Info about") + juce::JUCEApplication::getInstance()->getApplicationName());
        aboutMenu.showMenuAsync(juce::PopupMenu::Options());
    };
    addAndMakeVisible(m_aboutButton.get());

    m_powerButton = std::make_unique<juce::DrawableButton>("Quit application", juce::DrawableButton::ButtonStyle::ImageFitted);
    m_powerButton->setTooltip("Quit application");
    m_powerButton->onClick = [this] {
        juce::JUCEApplication::getInstance()->quit();
    };
    addAndMakeVisible(m_powerButton.get());

    m_emptySpace = std::make_unique<EmptySpace>();
    addAndMakeVisible(m_emptySpace.get());

    m_sysLoadBar = std::make_unique<LoadBar>("PLoad");
    addAndMakeVisible(m_sysLoadBar.get());

    m_netHealthBar = std::make_unique<LoadBar>("NLoad");
    addAndMakeVisible(m_netHealthBar.get());

    juce::Desktop::getInstance().addDarkModeSettingListener(this);
    darkModeSettingChanged(); // initially trigger correct colourscheme

}

MemaUIComponent::~MemaUIComponent()
{
    juce::Desktop::getInstance().removeDarkModeSettingListener(this);

    if (onDeleted)
        onDeleted();
}


void MemaUIComponent::setStandaloneWindow(bool standalone)
{
    m_isStandaloneWindow = standalone;

    if (standalone)
    {
        int styleFlags = juce::ComponentPeer::windowHasDropShadow
            | juce::ComponentPeer::windowAppearsOnTaskbar
            | juce::ComponentPeer::windowHasTitleBar;

        setVisible(true);
        addToDesktop(styleFlags);

        lookAndFeelChanged(); // trigger lookandfeel change to update icon (dock vs undock)
    }
    else
    {
        removeFromDesktop();
        setVisible(false);
    }
}

bool MemaUIComponent::isStandaloneWindow()
{
    return m_isStandaloneWindow;
}

void MemaUIComponent::setEditorComponent(juce::Component* editorComponent)
{
    if (m_editorComponent)
        removeChildComponent(m_editorComponent);

    m_editorComponent = editorComponent;

    addAndMakeVisible(editorComponent);
}

void MemaUIComponent::handleEditorSizeChangeRequest(const juce::Rectangle<int>& requestedSize)
{
    auto width = requestedSize.getWidth();
    auto height = requestedSize.getHeight() + sc_buttonSize;

    if (width < (2 * sc_loadNetWidth + 5 * sc_buttonSize))
        width = 2 * sc_loadNetWidth + 5 * sc_buttonSize;

    setSize(width, height);
}

void MemaUIComponent::updateCpuUsageBar(int loadPercent)
{
    if (m_sysLoadBar)
        m_sysLoadBar->setLoadPercent(loadPercent);
}

void MemaUIComponent::updateNetworkUsage(std::map<int, std::pair<double, bool>> netLoads)
{
    if (m_netHealthBar)
    {
        for (auto const& netLoad : netLoads)
        {
            m_netHealthBar->setLoadPercent(int(netLoad.second.first * 100.0), netLoad.first);
            m_netHealthBar->setAlert(netLoad.second.second, netLoad.first);
        }
    }
}

void MemaUIComponent::paint(Graphics &g)
{
    g.fillAll(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::defaultFill));
}

void MemaUIComponent::resized()
{
    auto safeBounds = getLocalBounds();

    auto margin = 1;
    auto setupElementArea = safeBounds.removeFromTop(sc_buttonSize);
    auto contentAreaBounds = safeBounds;
    contentAreaBounds.removeFromTop(1);

    // buttons from right
    if (m_powerButton)
        m_powerButton->setBounds(setupElementArea.removeFromRight(setupElementArea.getHeight()));
    setupElementArea.removeFromRight(margin);
    if (m_aboutButton)
        m_aboutButton->setBounds(setupElementArea.removeFromRight(setupElementArea.getHeight()));
    setupElementArea.removeFromRight(margin);
    if (m_appSettingsButton)
        m_appSettingsButton->setBounds(setupElementArea.removeFromRight(setupElementArea.getHeight()));
    setupElementArea.removeFromRight(margin);
    if (m_audioSettingsButton)
        m_audioSettingsButton->setBounds(setupElementArea.removeFromRight(setupElementArea.getHeight()));
    setupElementArea.removeFromRight(margin);
    if (m_toggleStandaloneWindowButton)
        m_toggleStandaloneWindowButton->setBounds(setupElementArea.removeFromRight(setupElementArea.getHeight()));
    setupElementArea.removeFromRight(margin);

    // load bars from left
    if (m_sysLoadBar)
        m_sysLoadBar->setBounds(setupElementArea.removeFromLeft(sc_loadNetWidth));
    setupElementArea.removeFromLeft(margin);
    if (m_netHealthBar)
        m_netHealthBar->setBounds(setupElementArea.removeFromLeft(sc_loadNetWidth));
    setupElementArea.removeFromLeft(margin);

    // correct-background spacing inbetween
    if (m_emptySpace)
        m_emptySpace->setBounds(setupElementArea);

    if (m_editorComponent)
        m_editorComponent->setBounds(contentAreaBounds);
}

void MemaUIComponent::darkModeSettingChanged()
{
    if (!m_followLocalStyle)
        return;

    if (juce::Desktop::getInstance().isDarkModeActive())
    {
        // go dark
        applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PS_Dark);
    }
    else
    {
        // go light
        applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PS_Light);
    }

    lookAndFeelChanged();
}

void MemaUIComponent::applyPaletteStyle(const JUCEAppBasics::CustomLookAndFeel::PaletteStyle& paletteStyle)
{
    m_lookAndFeel = std::make_unique<JUCEAppBasics::CustomLookAndFeel>(paletteStyle);
    juce::Desktop::getInstance().setDefaultLookAndFeel(m_lookAndFeel.get());
}

void MemaUIComponent::lookAndFeelChanged()
{
    auto powerDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::power_settings_24dp_svg).get());
    powerDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_powerButton->setImages(powerDrawable.get());

    auto appSettingsButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::settings_24dp_svg).get());
    appSettingsButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_appSettingsButton->setImages(appSettingsButtonDrawable.get());

    auto audioSettingsButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::tune_24dp_svg).get());
    audioSettingsButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_audioSettingsButton->setImages(audioSettingsButtonDrawable.get());

    auto aboutButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::question_mark_24dp_svg).get());
    aboutButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_aboutButton->setImages(aboutButtonDrawable.get());

    auto standaloneWindowDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse((isStandaloneWindow() ? BinaryData::open_in_new_down_24dp_svg : BinaryData::open_in_new24px_svg)).get());
    standaloneWindowDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
    m_toggleStandaloneWindowButton->setImages(standaloneWindowDrawable.get());
    
    if (onLookAndFeelChanged)
        onLookAndFeelChanged();

    applyMeteringColour();
}

std::unique_ptr<XmlElement> MemaUIComponent::createStateXml()
{
    auto stateXml = std::make_unique<juce::XmlElement>(MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::UICONFIG));

    int paletteStyle = -1;
    if (!m_followLocalStyle && m_lookAndFeel)
    {
        if (auto customLAF = dynamic_cast<JUCEAppBasics::CustomLookAndFeel*>(m_lookAndFeel.get()))
            paletteStyle = customLAF->getPaletteStyle();
    }
    stateXml->setAttribute(MemaAppConfiguration::getAttributeName(MemaAppConfiguration::AttributeID::PALETTESTYLE), paletteStyle);
    stateXml->setAttribute(MemaAppConfiguration::getAttributeName(MemaAppConfiguration::AttributeID::METERINGCOLOR), m_meteringColour.toString());

    return stateXml;
}

bool MemaUIComponent::setStateXml(XmlElement* stateXml)
{
    if (!stateXml || (stateXml->getTagName() != MemaAppConfiguration::getTagName(MemaAppConfiguration::TagID::UICONFIG)))
        return false;

    auto paletteStyle = stateXml->getIntAttribute(MemaAppConfiguration::getAttributeName(MemaAppConfiguration::AttributeID::PALETTESTYLE), -1);
    m_followLocalStyle = (-1 == paletteStyle);
    if (m_followLocalStyle)
        darkModeSettingChanged();
    else
        applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PaletteStyle(paletteStyle));
    m_settingsItems[MemaSettingsOption::LookAndFeel_Automatic] = std::make_pair("Automatic", -1 == paletteStyle);
    m_settingsItems[MemaSettingsOption::LookAndFeel_Dark] = std::make_pair("Dark", JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Dark == paletteStyle);
    m_settingsItems[MemaSettingsOption::LookAndFeel_Light] = std::make_pair("Light", JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Light == paletteStyle);

    auto meteringColour = juce::Colour::fromString(stateXml->getStringAttribute(MemaAppConfiguration::getAttributeName(MemaAppConfiguration::AttributeID::METERINGCOLOR)));
    setMeteringColour(meteringColour);
    m_settingsItems[MemaSettingsOption::MeteringColour_Green] = std::make_pair("Green", juce::Colours::forestgreen == meteringColour);
    m_settingsItems[MemaSettingsOption::MeteringColour_Red] = std::make_pair("Red", juce::Colours::orangered == meteringColour);
    m_settingsItems[MemaSettingsOption::MeteringColour_Blue] = std::make_pair("Blue", juce::Colours::dodgerblue == meteringColour);
    m_settingsItems[MemaSettingsOption::MeteringColour_Pink] = std::make_pair("Anni Pink", juce::Colours::deeppink == meteringColour);

    return true;
}

void MemaUIComponent::handleSettingsMenuResult(int selectedId)
{
    if (0 == selectedId)
        return; // nothing selected, dismiss
    else if (MemaSettingsOption::LookAndFeel_First <= selectedId && MemaSettingsOption::LookAndFeel_Last >= selectedId)
        handleSettingsLookAndFeelMenuResult(selectedId);
    else if (MemaSettingsOption::MeteringColour_First <= selectedId && MemaSettingsOption::MeteringColour_Last >= selectedId)
        handleSettingsMeteringColourMenuResult(selectedId);
    else
        jassertfalse; // unhandled menu entry!?

    if (onSettingsChanged)
        onSettingsChanged();
}

void MemaUIComponent::handleSettingsLookAndFeelMenuResult(int selectedId)
{
    // helper internal function to avoid code clones
    std::function<void(int, int, int)> setSettingsItemsCheckState = [=](int a, int b, int c) {
        m_settingsItems[MemaSettingsOption::LookAndFeel_Automatic].second = a;
        m_settingsItems[MemaSettingsOption::LookAndFeel_Dark].second = b;
        m_settingsItems[MemaSettingsOption::LookAndFeel_Light].second = c;
        };

    switch (selectedId)
    {
    case MemaSettingsOption::LookAndFeel_Automatic:
        setSettingsItemsCheckState(1, 0, 0);
        m_followLocalStyle = true;
        darkModeSettingChanged();
        break;
    case MemaSettingsOption::LookAndFeel_Dark:
        setSettingsItemsCheckState(0, 1, 0);
        m_followLocalStyle = false;
        applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Dark);
        break;
    case MemaSettingsOption::LookAndFeel_Light:
        setSettingsItemsCheckState(0, 0, 1);
        m_followLocalStyle = false;
        applyPaletteStyle(JUCEAppBasics::CustomLookAndFeel::PaletteStyle::PS_Light);
        break;
    default:
        jassertfalse; // unknown id fed in unintentionally ?!
        break;
    }
}

void MemaUIComponent::handleSettingsMeteringColourMenuResult(int selectedId)
{
    // helper internal function to avoid code clones
    std::function<void(int, int, int, int)> setSettingsItemsCheckState = [=](int green, int red, int blue, int pink) {
        m_settingsItems[MemaSettingsOption::MeteringColour_Green].second = green;
        m_settingsItems[MemaSettingsOption::MeteringColour_Red].second = red;
        m_settingsItems[MemaSettingsOption::MeteringColour_Blue].second = blue;
        m_settingsItems[MemaSettingsOption::MeteringColour_Pink].second = pink;
        };

    switch (selectedId)
    {
    case MemaSettingsOption::MeteringColour_Green:
        setSettingsItemsCheckState(1, 0, 0, 0);
        setMeteringColour(juce::Colours::forestgreen);
        break;
    case MemaSettingsOption::MeteringColour_Red:
        setSettingsItemsCheckState(0, 1, 0, 0);
        setMeteringColour(juce::Colours::orangered);
        break;
    case MemaSettingsOption::MeteringColour_Blue:
        setSettingsItemsCheckState(0, 0, 1, 0);
        setMeteringColour(juce::Colours::dodgerblue);
        break;
    case MemaSettingsOption::MeteringColour_Pink:
        setSettingsItemsCheckState(0, 0, 0, 1);
        setMeteringColour(juce::Colours::deeppink);
        break;
    default:
        break;
    }
}

void MemaUIComponent::setMeteringColour(const juce::Colour& meteringColour)
{
    m_meteringColour = meteringColour;

    applyMeteringColour();
}

void MemaUIComponent::applyMeteringColour()
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

} // namespace Mema

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


namespace Mema
{

//==============================================================================
class Spacing : public juce::Component
{
public:
    Spacing() = default;
    ~Spacing() = default;

    //==============================================================================
    void paint(Graphics& g) override
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    };
};

//==============================================================================
class PluginListAndSelectComponent : public juce::Component
{
public:
    PluginListAndSelectComponent()
    {
        m_formatManager.addDefaultFormats();
        juce::File deadMansPedalFile;
        juce::PropertiesFile::Options options;
        juce::PropertiesFile propertiesFile(options);
        m_pluginListComponent = std::make_unique<juce::PluginListComponent>(m_formatManager, m_pluginList, deadMansPedalFile, &propertiesFile);
        m_pluginListComponent->getTableListBox().setMultipleSelectionEnabled(false);
        addAndMakeVisible(m_pluginListComponent.get());

        m_selectButton = std::make_unique<juce::TextButton>("Select", "Accept the current plugin selection as new type to instantiate and close.");
        m_selectButton->onClick = [=]() {
            if (m_pluginListComponent)
            {
                jassert(1 == m_pluginListComponent->getTableListBox().getSelectedRows().size());
            }
            removeFromDesktop();
        };
        m_selectButton->setEnabled(false);
        addAndMakeVisible(m_selectButton.get());

        m_cancelButton = std::make_unique<juce::TextButton>("Cancel", "Discard the current plugin selection and close.");
        m_cancelButton->onClick = [=]() {
            removeFromDesktop();
        };
        addAndMakeVisible(m_cancelButton.get());

        setSize(885, 600);
    };
    ~PluginListAndSelectComponent() = default;

    //==============================================================================
    void paint(Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    };
    void resized() override
    {
        auto bounds = getLocalBounds();
        m_pluginListComponent->setBounds(bounds);

        bounds = bounds.removeFromBottom(28);
        m_cancelButton->setBounds(bounds.removeFromRight(80).reduced(2));
        m_selectButton->setBounds(bounds.removeFromRight(80).reduced(2));
    };

    //==============================================================================
    std::function<void(const juce::PluginDescription&)> onPluginSelected;

private:
    //==============================================================================
    juce::AudioPluginFormatManager              m_formatManager;
    juce::KnownPluginList                       m_pluginList;

    std::unique_ptr<juce::PluginListComponent>  m_pluginListComponent;
    std::unique_ptr<juce::TextButton>           m_selectButton;
    std::unique_ptr<juce::TextButton>           m_cancelButton;
};

//==============================================================================
class PluginControlComponent :   public juce::Component
{
public:
    PluginControlComponent();
    ~PluginControlComponent();

    void showPluginsList(juce::Point<int> showPosition);

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;

    //==============================================================================
    std::function<void(const juce::PluginDescription&)> onPluginSelected;
    std::function<void()> onShowPluginEditor;
    std::function<void(bool)> onPluginEnabledChange;

private:
    //==============================================================================
    std::unique_ptr<juce::DrawableButton>   m_enableButton;
    std::unique_ptr<Spacing>                m_spacing1;
    std::unique_ptr<juce::TextButton>       m_showEditorButton;
    std::unique_ptr<juce::DrawableButton>   m_triggerSelectButton;

    std::unique_ptr<PluginListAndSelectComponent>  m_pluginSelectionComponent;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginControlComponent)
};

}

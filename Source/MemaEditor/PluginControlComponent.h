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


/**
 * This class was stolen from juce's juce::PluginListComponent::TableModel
 * for customization to be able to react on row selection changes
 */
class CustomPluginListComponentTableModel : public juce::TableListBoxModel
{
public:
    CustomPluginListComponentTableModel(juce::PluginListComponent& c, juce::KnownPluginList& l) : m_owner(c), m_list(l) {}

    int getNumRows() override
    {
        return m_list.getNumTypes() + m_list.getBlacklistedFiles().size();
    }

    void paintRowBackground(juce::Graphics& g, int /*rowNumber*/, int /*width*/, int /*height*/, bool rowIsSelected) override
    {
        const auto defaultColour = m_owner.findColour(ListBox::backgroundColourId);
        const auto c = rowIsSelected ? defaultColour.interpolatedWith(m_owner.findColour(ListBox::textColourId), 0.5f)
            : defaultColour;

        g.fillAll(c);
    }

    enum
    {
        nameCol = 1,
        typeCol = 2,
        categoryCol = 3,
        manufacturerCol = 4,
        descCol = 5
    };

    void paintCell(juce::Graphics& g, int row, int columnId, int width, int height, bool /*rowIsSelected*/) override
    {
        juce::String text;
        bool isBlacklisted = row >= m_list.getNumTypes();

        if (isBlacklisted)
        {
            if (columnId == nameCol)
                text = m_list.getBlacklistedFiles()[row - m_list.getNumTypes()];
            else if (columnId == descCol)
                text = TRANS("Deactivated after failing to initialise correctly");
        }
        else
        {
            auto desc = m_list.getTypes()[row];

            switch (columnId)
            {
            case nameCol:         text = desc.name; break;
            case typeCol:         text = desc.pluginFormatName; break;
            case categoryCol:     text = desc.category.isNotEmpty() ? desc.category : "-"; break;
            case manufacturerCol: text = desc.manufacturerName; break;
            case descCol:         text = getPluginDescription(desc); break;

            default: jassertfalse; break;
            }
        }

        if (text.isNotEmpty())
        {
            const auto defaultTextColour = m_owner.findColour(ListBox::textColourId);
            g.setColour(isBlacklisted ? Colours::red
                : columnId == nameCol ? defaultTextColour
                : defaultTextColour.interpolatedWith(Colours::transparentBlack, 0.3f));
            g.setFont(m_owner.withDefaultMetrics(juce::FontOptions((float)height * 0.7f, Font::bold)));
            g.drawFittedText(text, 4, 0, width - 6, height, juce::Justification::centredLeft, 1, 0.9f);
        }
    }

    void cellClicked(int rowNumber, int columnId, const juce::MouseEvent& e) override
    {
        juce::TableListBoxModel::cellClicked(rowNumber, columnId, e);

        if (rowNumber >= 0 && rowNumber < getNumRows() && e.mods.isPopupMenu())
            m_owner.createMenuForRow(rowNumber).showMenuAsync(juce::PopupMenu::Options().withDeletionCheck(m_owner));
    }

    void deleteKeyPressed(int) override
    {
        m_owner.removeSelectedPlugins();
    }

    void sortOrderChanged(int newSortColumnId, bool isForwards) override
    {
        switch (newSortColumnId)
        {
        case nameCol:         m_list.sort(juce::KnownPluginList::sortAlphabetically, isForwards); break;
        case typeCol:         m_list.sort(juce::KnownPluginList::sortByFormat, isForwards); break;
        case categoryCol:     m_list.sort(juce::KnownPluginList::sortByCategory, isForwards); break;
        case manufacturerCol: m_list.sort(juce::KnownPluginList::sortByManufacturer, isForwards); break;
        case descCol:         break;

        default: jassertfalse; break;
        }
    }

    void selectedRowsChanged(int lastRowSelected) override
    {
        m_lastRowSelected = lastRowSelected;

        juce::TableListBoxModel::selectedRowsChanged(lastRowSelected);

        if (onSelectionChanged)
            onSelectionChanged(m_lastRowSelected);
    }

    static juce::String getPluginDescription(const juce::PluginDescription& desc)
    {
        juce::StringArray items;

        if (desc.descriptiveName != desc.name)
            items.add(desc.descriptiveName);

        items.add(desc.version);

        items.removeEmptyStrings();
        return items.joinIntoString(" - ");
    }

    juce::PluginDescription getPluginDescriptionOfSelectedRow()
    {
        if (m_lastRowSelected != -1)
        {
            auto currentPluginDescriptionListContents = m_list.getTypes();
            if (currentPluginDescriptionListContents.size() > m_lastRowSelected)
                return currentPluginDescriptionListContents[m_lastRowSelected];
        }
        
        return {};
    }

    std::function<void(int)> onSelectionChanged;

    //==============================================================================
    juce::PluginListComponent&  m_owner;
    juce::KnownPluginList&      m_list;
    int                         m_lastRowSelected = -1;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomPluginListComponentTableModel)
};

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


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Spacing)
};

//==============================================================================
class PluginListAndSelectComponent : public juce::Component
{
public:
    PluginListAndSelectComponent()
    {
        m_formatManager.addDefaultFormats();
        juce::File deadMansPedalFile;
        m_pluginListComponent = std::make_unique<juce::PluginListComponent>(m_formatManager, m_pluginList, deadMansPedalFile, nullptr);
        m_pluginListComponent->getTableListBox().setMultipleSelectionEnabled(false);
        auto customTableModel = std::make_unique<CustomPluginListComponentTableModel>(*m_pluginListComponent, m_pluginList);
        customTableModel->onSelectionChanged = [=](int lastRowSelected) {
            if (m_selectButton)
                m_selectButton->setEnabled(-1 != lastRowSelected);
        };
        m_pluginListComponent->setTableModel(dynamic_cast<juce::TableListBoxModel*>(customTableModel.release()));
        addAndMakeVisible(m_pluginListComponent.get());

        m_selectButton = std::make_unique<juce::TextButton>("Select", "Accept the current plugin selection as new type to instantiate and close.");
        m_selectButton->onClick = [=]() {
            if (m_pluginListComponent)
            {
                auto customTableModel = dynamic_cast<CustomPluginListComponentTableModel*>(m_pluginListComponent->getTableListBox().getTableListBoxModel());
                if (customTableModel && onPluginSelected)
                    onPluginSelected(customTableModel->getPluginDescriptionOfSelectedRow());
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


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginListAndSelectComponent)
};

//==============================================================================
class PluginControlComponent :   public juce::Component
{
public:
    PluginControlComponent();
    ~PluginControlComponent();

    void showPluginsList(juce::Point<int> showPosition);
    void setSelectedPlugin(const juce::PluginDescription& pluginDescription);

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void lookAndFeelChanged() override;

    //==============================================================================
    std::function<void(const juce::PluginDescription&)> onPluginSelected;
    std::function<void()>                               onShowPluginEditor;
    std::function<void(bool)>                           onPluginEnabledChange;
    std::function<void()>                               onClearPlugin;

private:
    //==============================================================================
    std::unique_ptr<juce::DrawableButton>   m_enableButton;
    std::unique_ptr<Spacing>                m_spacing1;
    std::unique_ptr<juce::TextButton>       m_showEditorButton;
    std::unique_ptr<juce::DrawableButton>   m_triggerSelectButton;
    std::unique_ptr<Spacing>                m_spacing2;
    std::unique_ptr<juce::DrawableButton>   m_clearButton;

    std::unique_ptr<PluginListAndSelectComponent>   m_pluginSelectionComponent;
    juce::PluginDescription                         m_selectedPluginDescription;


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginControlComponent)
};

}

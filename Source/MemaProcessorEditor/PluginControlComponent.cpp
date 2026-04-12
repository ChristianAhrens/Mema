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

#include "PluginControlComponent.h"


namespace Mema
{

//==============================================================================
// ParameterRowComponent
//==============================================================================

ParameterRowComponent::ParameterRowComponent(int paramIdx, const Mema::PluginParameterInfo& info)
    : paramIndex(paramIdx)
{
    toggleButton = std::make_unique<juce::ToggleButton>(info.name);
    toggleButton->setToggleState(info.isRemoteControllable, juce::dontSendNotification);
    addAndMakeVisible(toggleButton.get());

    typeCombo = std::make_unique<juce::ComboBox>();
    typeCombo->addItem("Continuous", static_cast<int>(ParameterControlType::Continuous) + 1);
    typeCombo->addItem("Discrete",   static_cast<int>(ParameterControlType::Discrete) + 1);
    typeCombo->addItem("Toggle",     static_cast<int>(ParameterControlType::Toggle) + 1);
    typeCombo->setSelectedId(static_cast<int>(info.type) + 1, juce::dontSendNotification);
    addAndMakeVisible(typeCombo.get());

    stepsEdit = std::make_unique<JUCEAppBasics::FixedFontTextEditor>();
    stepsEdit->setText(info.type == ParameterControlType::Toggle ? juce::String(2) : juce::String(info.stepCount), juce::dontSendNotification);
    stepsEdit->setEnabled(info.type != ParameterControlType::Toggle);
    stepsEdit->setInputRestrictions(3, "0123456789");
    addAndMakeVisible(stepsEdit.get());

    typeCombo->onChange = [this]() {
        auto selectedType = static_cast<ParameterControlType>(typeCombo->getSelectedId() - 1);
        stepsEdit->setEnabled(selectedType != ParameterControlType::Toggle);
        if (selectedType == ParameterControlType::Toggle)
            stepsEdit->setText(juce::String(2), juce::dontSendNotification);
    };
}

void ParameterRowComponent::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromLeft(gripWidth);
    toggleButton->setBounds(bounds.removeFromLeft(160));
    typeCombo->setBounds(bounds.removeFromLeft(100));
    stepsEdit->setBounds(bounds);
}

void ParameterRowComponent::paint(juce::Graphics& g)
{
    // Grip dots (2 columns × 3 rows)
    const float dotSize  = 2.0f;
    const float colGap   = 5.0f;
    const float rowGap   = 5.0f;
    const float startX   = (gripWidth - colGap - dotSize) * 0.5f;
    const float startY   = (getHeight() - 2.0f * rowGap - dotSize) * 0.5f;
    g.setColour(findColour(juce::Label::textColourId).withAlpha(0.4f));
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 2; ++c)
            g.fillEllipse(startX + c * colGap, startY + r * rowGap, dotSize, dotSize);

    // Insertion indicator drawn at the top or bottom edge
    if (m_showInsertionLine)
    {
        g.setColour(findColour(juce::TextButton::ColourIds::textColourOnId));
        g.fillRect(0, m_insertionLineAtTop ? 0 : getHeight() - 2, getWidth(), 2);
    }
}

void ParameterRowComponent::mouseDown(const juce::MouseEvent& e)
{
    m_mouseDownInGrip = (e.x < gripWidth);
}

void ParameterRowComponent::mouseDrag(const juce::MouseEvent& e)
{
    if (m_mouseDownInGrip)
    {
        if (auto* container = juce::DragAndDropContainer::findParentDragContainerFor(this))
            if (!container->isDragAndDropActive())
                container->startDragging(juce::var(paramIndex), this);
    }
}

void ParameterRowComponent::mouseUp(const juce::MouseEvent&)
{
    m_mouseDownInGrip = false;
}

bool ParameterRowComponent::isInterestedInDragSource(const SourceDetails& details)
{
    auto* source = dynamic_cast<ParameterRowComponent*>(details.sourceComponent.get());
    return source != nullptr && source->paramIndex != paramIndex;
}

void ParameterRowComponent::itemDragEnter(const SourceDetails& details)
{
    m_showInsertionLine  = true;
    m_insertionLineAtTop = (details.localPosition.y < getHeight() / 2);
    repaint();
}

void ParameterRowComponent::itemDragMove(const SourceDetails& details)
{
    bool atTop = (details.localPosition.y < getHeight() / 2);
    if (atTop != m_insertionLineAtTop)
    {
        m_insertionLineAtTop = atTop;
        repaint();
    }
}

void ParameterRowComponent::itemDragExit(const SourceDetails&)
{
    m_showInsertionLine = false;
    repaint();
}

void ParameterRowComponent::itemDropped(const SourceDetails& details)
{
    auto* source = dynamic_cast<ParameterRowComponent*>(details.sourceComponent.get());
    if (source && onRowDropped)
        onRowDropped(source->paramIndex, paramIndex, details.localPosition.y < getHeight() / 2);
    m_showInsertionLine = false;
    repaint();
}

//==============================================================================
// ParameterListComponent
//==============================================================================

void ParameterListComponent::addRow(std::unique_ptr<ParameterRowComponent> row)
{
    row->onRowDropped = [this](int from, int to, bool before) {
        reorderRow(from, to, before);
    };
    addAndMakeVisible(row.get());
    m_rows.push_back(std::move(row));
    layoutRows();
}

void ParameterListComponent::layoutRows()
{
    const int rowHeight = 28;
    for (int i = 0; i < static_cast<int>(m_rows.size()); ++i)
        m_rows[i]->setBounds(0, i * rowHeight, getWidth(), rowHeight);
}

std::vector<int> ParameterListComponent::getDisplayOrder() const
{
    std::vector<int> order;
    order.reserve(m_rows.size());
    for (auto const& row : m_rows)
        order.push_back(row->paramIndex);
    return order;
}

ParameterRowComponent* ParameterListComponent::getRowForParamIndex(int paramIdx)
{
    for (auto& row : m_rows)
        if (row->paramIndex == paramIdx)
            return row.get();
    return nullptr;
}

void ParameterListComponent::reorderRow(int fromParamIndex, int toParamIndex, bool insertBefore)
{
    int fromIdx = -1, toIdx = -1;
    for (int i = 0; i < static_cast<int>(m_rows.size()); ++i)
    {
        if (m_rows[i]->paramIndex == fromParamIndex) fromIdx = i;
        if (m_rows[i]->paramIndex == toParamIndex)   toIdx   = i;
    }

    if (fromIdx < 0 || toIdx < 0)
        return;

    int insertIdx = insertBefore ? toIdx : toIdx + 1;

    // Adjust insertion index for the removal of the source row
    if (insertIdx > fromIdx)
        insertIdx--;

    if (insertIdx == fromIdx)
        return;

    auto movedRow = std::move(m_rows[fromIdx]);
    m_rows.erase(m_rows.begin() + fromIdx);
    m_rows.insert(m_rows.begin() + insertIdx, std::move(movedRow));

    layoutRows();
}

//==============================================================================
PluginControlComponent::PluginControlComponent()
    : juce::Component()
{
	m_enableButton = std::make_unique<juce::DrawableButton>("Enable plug-in", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_enableButton->setTooltip("Enable plug-in");
	m_enableButton->setClickingTogglesState(true);
	m_enableButton->onClick = [this] { 
		if (onPluginEnabledChange)
			onPluginEnabledChange(m_enableButton->getToggleState());
	};
	addAndMakeVisible(m_enableButton.get());

	m_spacing1 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing1.get());

	m_postButton = std::make_unique<juce::TextButton>("Post", "Toggle plug-in pre/post");
	m_postButton->setClickingTogglesState(true);
	m_postButton->onClick = [this] {
		if (onPluginPrePostChange)
			onPluginPrePostChange(m_postButton->getToggleState());
		};
	addAndMakeVisible(m_postButton.get());

	m_spacing2 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing2.get());

	m_showEditorButton = std::make_unique<juce::TextButton>("None", "Show plug-in editor");
	m_showEditorButton->onClick = [this] {
		if (onShowPluginEditor)
			onShowPluginEditor();
	};
	addAndMakeVisible(m_showEditorButton.get());

	m_triggerSelectButton = std::make_unique<juce::DrawableButton>("Show plug-in selection menu", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_triggerSelectButton->setTooltip("Show plug-in selection menu");
	m_triggerSelectButton->onClick = [this] {
		showPluginsList(juce::Desktop::getMousePosition());
	};
	addAndMakeVisible(m_triggerSelectButton.get());

	m_spacing3 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing3.get());

	m_parameterConfigButton = std::make_unique<juce::DrawableButton>("Configure plug-in parameters", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_parameterConfigButton->setTooltip("Configure plug-in parameters");
	m_parameterConfigButton->onClick = [this] {
		showParameterConfig();
	};
	addAndMakeVisible(m_parameterConfigButton.get());

	m_spacing4 = std::make_unique<Spacing>();
	addAndMakeVisible(m_spacing4.get());

	m_clearButton = std::make_unique<juce::DrawableButton>("Clear current plug-in", juce::DrawableButton::ButtonStyle::ImageOnButtonBackground);
	m_clearButton->setTooltip("Clear current plug-in");
	m_clearButton->onClick = [this] {
		if (onClearPlugin)
			onClearPlugin();
		};
	addAndMakeVisible(m_clearButton.get());

	m_pluginSelectionComponent = std::make_unique<PluginListAndSelectComponent>();
	m_pluginSelectionComponent->onPluginSelected = [=](const juce::PluginDescription& pluginDescription) {
		if (onPluginSelected)
			onPluginSelected(pluginDescription);
	};
}

PluginControlComponent::~PluginControlComponent()
{

}

void PluginControlComponent::showPluginsList(juce::Point<int> showPosition)
{
	m_pluginSelectionComponent->setVisible(true);
	m_pluginSelectionComponent->addToDesktop(juce::ComponentPeer::windowHasDropShadow);

	auto const display = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay();
	if (nullptr != display && nullptr != m_pluginSelectionComponent)
	{
		if (display->totalArea.getHeight() < showPosition.getY() + m_pluginSelectionComponent->getHeight())
			showPosition.setY(showPosition.getY() - m_pluginSelectionComponent->getHeight() - 30);
		if (display->totalArea.getWidth() < showPosition.getX() + m_pluginSelectionComponent->getWidth())
			showPosition.setX(showPosition.getX() - m_pluginSelectionComponent->getWidth() - 30);
	}
	m_pluginSelectionComponent->setTopLeftPosition(showPosition);
}

void PluginControlComponent::setPluginEnabled(bool enabled)
{
	if (m_enableButton)
		m_enableButton->setToggleState(enabled, juce::dontSendNotification);
}

void PluginControlComponent::setPluginPrePost(bool post)
{
	if (m_postButton)
		m_postButton->setToggleState(post, juce::dontSendNotification);
}

void PluginControlComponent::setSelectedPlugin(const juce::PluginDescription& pluginDescription)
{
	m_selectedPluginDescription = pluginDescription;

	if (m_showEditorButton)
	{
		if (m_selectedPluginDescription.name.isEmpty())
			m_showEditorButton->setButtonText("None");
		else
			m_showEditorButton->setButtonText(m_selectedPluginDescription.name);
	}
}

void PluginControlComponent::setParameterInfos(const std::vector<Mema::PluginParameterInfo>& infos)
{
	if (infos.size() != m_parameterInfos.size())
	{
		m_parameterInfos.clear();
		m_parameterDisplayOrder.clear();
	}

	auto key = 0;
	for (auto const& info : infos)
	{
		if (0 < m_parameterInfos.count(key) || std::as_const(m_parameterInfos[key]) != info)
			m_parameterInfos[key] = info;
		key++;
	}
}

const std::map<int, Mema::PluginParameterInfo>& PluginControlComponent::getParameterInfos()
{
	return m_parameterInfos;
}

void PluginControlComponent::setParameterDisplayOrder(const std::vector<int>& order)
{
	m_parameterDisplayOrder = order;
}

const std::vector<int>& PluginControlComponent::getParameterDisplayOrder()
{
	if (m_parameterDisplayOrder.empty())
		for (auto& kv : m_parameterInfos)
			m_parameterDisplayOrder.push_back(kv.first);
	return m_parameterDisplayOrder;
}

void PluginControlComponent::showParameterConfig()
{
	if (m_selectedPluginDescription.name.isEmpty())
	{
		m_messageBox = std::make_unique<juce::AlertWindow>(
			"Plug-in parameter setup not available",
			"No plug-in selected.",
			juce::MessageBoxIconType::WarningIcon);
		m_messageBox->addButton("Ok", 1, juce::KeyPress(juce::KeyPress::returnKey));
		m_messageBox->enterModalState(true, juce::ModalCallbackFunction::create([=](int returnValue) {
			ignoreUnused(returnValue);
			m_messageBox.reset();
			}));
	}
	else if (m_parameterInfos.empty())
	{
		m_messageBox = std::make_unique<juce::AlertWindow>(
			"Plug-in parameter setup not available",
			"No parameters detected.",
			juce::MessageBoxIconType::WarningIcon);
		m_messageBox->addButton("Ok", 1, juce::KeyPress(juce::KeyPress::returnKey));
		m_messageBox->enterModalState(true, juce::ModalCallbackFunction::create([=](int returnValue) {
			ignoreUnused(returnValue);
			m_messageBox.reset();
			}));
	}
	else
	{
		m_messageBox = std::make_unique<juce::AlertWindow>(
			"Plug-in parameter setup",
			"Select which parameters should be remote-controllable and configure their control type.\nDrag the grip handle on the left of each row to reorder.",
			juce::MessageBoxIconType::NoIcon);

		// Initialise display order from natural key order if not yet set
		if (m_parameterDisplayOrder.empty())
			for (auto& kv : m_parameterInfos)
				m_parameterDisplayOrder.push_back(kv.first);

		// Save for cancel restore
		auto savedDisplayOrder = m_parameterDisplayOrder;

		const int rowHeight = 28;
		const int totalWidth = ParameterRowComponent::gripWidth + 160 + 100 + 60; // grip + toggle + combo + steps
		const int totalHeight = static_cast<int>(m_parameterInfos.size()) * rowHeight;

		// Build the drag-reorderable list in current display order
		m_messageBoxParameterListComponent = std::make_unique<ParameterListComponent>();
		m_messageBoxParameterListComponent->setSize(totalWidth, totalHeight);
		for (int paramIdx : m_parameterDisplayOrder)
			m_messageBoxParameterListComponent->addRow(
				std::make_unique<ParameterRowComponent>(paramIdx, m_parameterInfos.at(paramIdx)));

		// Wrap in a scrollable viewport capped to avoid overflowing the screen
		const int maxViewportHeight = 400;
		const int viewportHeight = juce::jmin(totalHeight, maxViewportHeight);

		m_messageBoxParameterTogglesViewport = std::make_unique<juce::Viewport>();
		m_messageBoxParameterTogglesViewport->setViewedComponent(m_messageBoxParameterListComponent.get(), false);
		m_messageBoxParameterTogglesViewport->setScrollBarsShown(totalHeight > maxViewportHeight, false);
		m_messageBoxParameterTogglesViewport->setSize(
			totalWidth + (totalHeight > maxViewportHeight ? m_messageBoxParameterTogglesViewport->getScrollBarThickness() : 0),
			viewportHeight);

		m_messageBox->addCustomComponent(m_messageBoxParameterTogglesViewport.get());
		m_messageBox->addButton("Cancel", 0, juce::KeyPress(juce::KeyPress::escapeKey));
		m_messageBox->addButton("Ok", 1, juce::KeyPress(juce::KeyPress::returnKey));

		m_messageBox->enterModalState(true, juce::ModalCallbackFunction::create([=](int returnValue) {
			if (returnValue == 1)
			{
				auto changeDetected = false;

				for (auto& parameterKV : m_parameterInfos)
				{
					auto paramIndex = parameterKV.first;
					auto& paramInfo = parameterKV.second;

					auto* row = m_messageBoxParameterListComponent->getRowForParamIndex(paramIndex);
					if (!row) continue;

					// Check remote controllable toggle
					auto newRemoteControllable = row->toggleButton->getToggleState();
					if (newRemoteControllable != paramInfo.isRemoteControllable)
					{
						paramInfo.isRemoteControllable = newRemoteControllable;
						changeDetected = true;
					}

					// Check control type
					auto newType = static_cast<ParameterControlType>(row->typeCombo->getSelectedId() - 1);
					if (newType != paramInfo.type)
					{
						paramInfo.type = newType;
						if (newType == ParameterControlType::Toggle)
							paramInfo.stepCount = 2;
						changeDetected = true;
					}

					// Check step count - ignored for toggle type
					if (newType != ParameterControlType::Toggle)
					{
						auto newStepCount = row->stepsEdit->getText().getIntValue();
						newStepCount = juce::jmax(2, newStepCount); // Enforce minimum of 2 steps
						if (newStepCount != paramInfo.stepCount)
						{
							paramInfo.stepCount = newStepCount;
							changeDetected = true;
						}
					}
				}

				auto newDisplayOrder = m_messageBoxParameterListComponent->getDisplayOrder();
				if (newDisplayOrder != savedDisplayOrder)
					changeDetected = true;
				m_parameterDisplayOrder = newDisplayOrder;

				if (changeDetected)
				{
					if (onPluginParametersStatusChanged)
						onPluginParametersStatusChanged();
				}
			}
			else
			{
				// Cancel: restore the display order to what it was before the dialog opened
				m_parameterDisplayOrder = savedDisplayOrder;
			}

			m_messageBoxParameterTogglesViewport.reset();
			m_messageBoxParameterListComponent.reset();
			m_messageBox.reset();
			}));
	}
}

void PluginControlComponent::resized()
{
    auto bounds = getLocalBounds();
	auto margin = 1;

	if (m_enableButton)
		m_enableButton->setBounds(bounds.removeFromLeft(bounds.getHeight()));
	if (m_spacing1)
		m_spacing1->setBounds(bounds.removeFromLeft(margin));
	if (m_postButton)
		m_postButton->setBounds(bounds.removeFromLeft(int(1.5f * bounds.getHeight())));
	if (m_spacing2)
		m_spacing2->setBounds(bounds.removeFromLeft(margin));
	if (m_clearButton)
		m_clearButton->setBounds(bounds.removeFromRight(bounds.getHeight()));
	if (m_spacing3)
		m_spacing3->setBounds(bounds.removeFromRight(margin));
	if (m_parameterConfigButton)
		m_parameterConfigButton->setBounds(bounds.removeFromRight(bounds.getHeight()));
	if (m_spacing4)
		m_spacing4->setBounds(bounds.removeFromRight(margin));
	if (m_triggerSelectButton)
		m_triggerSelectButton->setBounds(bounds.removeFromRight(bounds.getHeight()));
	if (m_showEditorButton)
		m_showEditorButton->setBounds(bounds);
}

void PluginControlComponent::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(juce::LookAndFeel_V4::ColourScheme::defaultFill));
}

void PluginControlComponent::lookAndFeelChanged()
{
	auto enableDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::power_settings_24dp_svg).get());
	enableDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
	m_enableButton->setImages(enableDrawable.get());

	auto triggerSelectDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::stat_minus_1_24dp_svg).get());
	triggerSelectDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
	m_triggerSelectButton->setImages(triggerSelectDrawable.get());

	auto parameterConfigButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::settings_24dp_svg).get());
	parameterConfigButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
	m_parameterConfigButton->setImages(parameterConfigButtonDrawable.get());

	auto clearButtonDrawable = juce::Drawable::createFromSVG(*juce::XmlDocument::parse(BinaryData::replay_24dp_svg).get());
	clearButtonDrawable->replaceColour(juce::Colours::black, getLookAndFeel().findColour(juce::TextButton::ColourIds::textColourOnId));
	m_clearButton->setImages(clearButtonDrawable.get());

	juce::Component::lookAndFeelChanged();
}


}

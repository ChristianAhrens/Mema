/* Copyright (c) 2025, Christian Ahrens
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

#include "MemaClientControlComponents.h"

#include <CustomLookAndFeel.h>
#include <ToggleStateSlider.h>
#include <MemaProcessor/ProcessorDataAnalyzer.h>
#include <MemaClientCommon/TwoDFieldMultisliderComponent.h>


namespace Mema
{


MemaClientControlComponentBase::MemaClientControlComponentBase()
    : juce::Component()
{
}

MemaClientControlComponentBase::~MemaClientControlComponentBase()
{
}

void MemaClientControlComponentBase::setControlsSize(const ControlsSize& ctrlsSize)
{
    m_controlsSize = ctrlsSize;
}

void MemaClientControlComponentBase::setIOCount(const std::pair<int, int>& ioCount)
{
    m_ioCount = ioCount;
}

const std::pair<int, int>& MemaClientControlComponentBase::getIOCount()
{
    return m_ioCount;
}

void MemaClientControlComponentBase::setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates)
{
    m_inputMuteStates = inputMuteStates;
}

const std::map<std::uint16_t, bool>& MemaClientControlComponentBase::getInputMuteStates()
{
    return m_inputMuteStates;
}

void MemaClientControlComponentBase::setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates)
{
    m_outputMuteStates = outputMuteStates;
}

const std::map<std::uint16_t, bool>& MemaClientControlComponentBase::getOutputMuteStates()
{
    return m_outputMuteStates;
}

void MemaClientControlComponentBase::setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates)
{
    m_crosspointStates = crosspointStates;
}

const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& MemaClientControlComponentBase::getCrosspointStates()
{
    return m_crosspointStates;
}

void MemaClientControlComponentBase::setCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues)
{
    //juce::String dbgStr;
    //for (auto const& iKV : crosspointValues)
    //{
    //    dbgStr << int(iKV.first) << " - ";
    //    for (auto const& oKV : iKV.second)
    //        dbgStr << int(oKV.first) << ":" << oKV.second << ";";
    //    dbgStr << "\n";
    //}
    //DBG(juce::String(__FUNCTION__) + "\n" + dbgStr);

    m_crosspointValues = crosspointValues;
}

const std::map<std::uint16_t, std::map<std::uint16_t, float>>& MemaClientControlComponentBase::getCrosspointValues()
{
    //juce::String dbgStr;
    //for (auto const& iKV : m_crosspointValues)
    //{
    //    dbgStr << int(iKV.first) << " - ";
    //    for (auto const& oKV : iKV.second)
    //        dbgStr << int(oKV.first) << ":" << oKV.second << ";";
    //    dbgStr << "\n";
    //}
    //DBG(juce::String(__FUNCTION__) + "\n" + dbgStr);

    return m_crosspointValues;
}

void MemaClientControlComponentBase::addCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates)
{
    auto crosspointStatesCpy = getCrosspointStates();

    for (auto const& iKV : crosspointStates)
    {
        auto& input = iKV.first;
        for (auto const& oKV : iKV.second)
        {
            auto& output = oKV.first;
            auto& state = oKV.second;
            crosspointStatesCpy[input][output] = state;
        }
    }
    setCrosspointStates(crosspointStatesCpy);
}

void MemaClientControlComponentBase::addCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues)
{
    auto crosspointValuesCpy = getCrosspointValues();

    for (auto const& iKV : crosspointValues)
    {
        auto& input = iKV.first;
        for (auto const& oKV : iKV.second)
        {
            auto& output = oKV.first;
            auto& value = oKV.second;
            crosspointValuesCpy[input][output] = value;
        }
    }
    setCrosspointValues(crosspointValuesCpy);
}

const juce::String MemaClientControlComponentBase::getClientControlParametersAsString()
{
    auto controlParametersStr = getIOCountParametersAsString() + "\n";
    controlParametersStr += getInputMuteParametersAsString() + "\n";
    controlParametersStr += getOutputMuteParametersAsString() + "\n";
    controlParametersStr += getCrosspointParametersAsString();
    return controlParametersStr;
}

const juce::String MemaClientControlComponentBase::getIOCountParametersAsString()
{
    auto controlParametersStr = juce::String("IO ");
    controlParametersStr << getIOCount().first << "x" << getIOCount().second;
    return controlParametersStr;
}

const juce::String MemaClientControlComponentBase::getInputMuteParametersAsString()
{
    auto controlParametersStr = juce::String("InputMutes: ");
    for (auto const& mutestate : getInputMuteStates())
        controlParametersStr << int(mutestate.first) << ":" << (mutestate.second ? "on" : "off") << ";";
    return controlParametersStr;
}

const juce::String MemaClientControlComponentBase::getOutputMuteParametersAsString()
{
    auto controlParametersStr = juce::String("OutputMutes: ");
    for (auto const& mutestate : getOutputMuteStates())
        controlParametersStr << int(mutestate.first) << ":" << (mutestate.second ? "on" : "off") << ";";
    return controlParametersStr;
}

const juce::String MemaClientControlComponentBase::getCrosspointParametersAsString()
{
    auto controlParametersStr = juce::String("Crosspoints:\n");
    auto crosspointValues = getCrosspointValues();
    for (auto const& crosspointstateFKV : getCrosspointStates())
    {
        auto& in = crosspointstateFKV.first;
        for (auto const& crosspointstateSKV : crosspointstateFKV.second)
        {
            auto& out = crosspointstateSKV.first;
            controlParametersStr << int(in) << "." << int(out) << ":" << (crosspointstateSKV.second ? "on" : "off") << "(" << crosspointValues[in][out] << ");";
        }
        controlParametersStr << "\n";
    }
    return controlParametersStr;
}


FaderbankControlComponent::FaderbankControlComponent()
    : MemaClientControlComponentBase()
{
    m_horizontalScrollContainerComponent = std::make_unique<juce::Component>();
    m_horizontalScrollViewport = std::make_unique<juce::Viewport>();
    m_horizontalScrollViewport->setViewedComponent(m_horizontalScrollContainerComponent.get(), false);
    addAndMakeVisible(m_horizontalScrollViewport.get());
    m_verticalScrollContainerComponent = std::make_unique<juce::Component>();
    m_verticalScrollViewport = std::make_unique<juce::Viewport>();
    m_verticalScrollViewport->setViewedComponent(m_verticalScrollContainerComponent.get(), false);
    addAndMakeVisible(m_verticalScrollViewport.get());

    m_inputControlsGrid = std::make_unique<juce::Grid>();
    m_inputControlsGrid->setGap(juce::Grid::Px(s_gap));

    m_outputControlsGrid = std::make_unique<juce::Grid>();
    m_outputControlsGrid->setGap(juce::Grid::Px(s_gap));

    m_crosspointsControlsGrid = std::make_unique<juce::Grid>();
    m_crosspointsControlsGrid->setGap(juce::Grid::Px(s_gap));
    m_crosspointsNoSelectionLabel = std::make_unique<juce::Label>("NoSelectLabel", "Select a channel to control crosspoints.");
    m_crosspointsNoSelectionLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(m_crosspointsNoSelectionLabel.get());
}

FaderbankControlComponent::~FaderbankControlComponent()
{
}

void FaderbankControlComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    auto ctrlsSize = 2 * (m_controlsSize + s_gap);
    g.setColour(getLookAndFeel().findColour(juce::Slider::backgroundColourId));
    g.fillRect(getLocalBounds().removeFromTop(ctrlsSize + s_scrollbarsize).removeFromRight(getWidth() - ctrlsSize));
    g.fillRect(getLocalBounds().removeFromLeft(ctrlsSize + s_scrollbarsize).removeFromBottom(getHeight() - ctrlsSize));
}

void FaderbankControlComponent::resized()
{
    auto ctrlsSize = 2 * (m_controlsSize + s_gap) + s_scrollbarsize;
    auto currentInputsWidth = (m_inputControlsGrid->getNumberOfColumns() * (m_controlsSize + s_gap)) - s_gap;
    auto currentOutputsHeight = (m_outputControlsGrid->getNumberOfRows() * (m_controlsSize + s_gap)) - s_gap;
    auto crosspointControlBounds = getLocalBounds();

    if (m_inputControlsGrid)
        m_inputControlsGrid->performLayout({ 0, 0, currentInputsWidth, ctrlsSize - s_scrollbarsize });

    if (m_outputControlsGrid)
        m_outputControlsGrid->performLayout({ 0, 0, ctrlsSize - s_scrollbarsize, currentOutputsHeight });

    if (m_crosspointsControlsGrid && m_currentIOChannel.first != ControlDirection::None)
    {
        if (ControlDirection::Input == m_currentIOChannel.first)
        {
            crosspointControlBounds.removeFromLeft(ctrlsSize);
            crosspointControlBounds.removeFromRight(s_gap);
        }
        else if (ControlDirection::Output == m_currentIOChannel.first)
        {
            crosspointControlBounds.removeFromTop(ctrlsSize);
            crosspointControlBounds.removeFromBottom(s_gap);
        }
        m_crosspointsControlsGrid->performLayout(crosspointControlBounds);
        m_crosspointsNoSelectionLabel->setVisible(false);
    }
    else if (m_currentIOChannel.first == ControlDirection::None)
    {
        for (auto const& item : m_crosspointsControlsGrid->items)
            if (nullptr != item.associatedComponent)
                item.associatedComponent->setVisible(false);
        crosspointControlBounds.removeFromTop(ctrlsSize);
        crosspointControlBounds.removeFromLeft(ctrlsSize);
        m_crosspointsNoSelectionLabel->setVisible(true);
        m_crosspointsNoSelectionLabel->setBounds(crosspointControlBounds);
    }

    if (ControlDirection::Input == m_currentIOChannel.first)
    {
        m_horizontalScrollContainerComponent->setBounds({ 0, 0, currentInputsWidth, ctrlsSize - s_scrollbarsize });
        m_horizontalScrollViewport->setBounds(getLocalBounds().removeFromTop(ctrlsSize).removeFromRight(getWidth() - ctrlsSize));
        m_verticalScrollContainerComponent->setBounds({ 0, 0, getWidth() - s_scrollbarsize, currentOutputsHeight });
        m_verticalScrollViewport->setBounds(getLocalBounds().removeFromBottom(getHeight() - ctrlsSize));
    }
    else if (ControlDirection::Output == m_currentIOChannel.first)
    {
        m_horizontalScrollContainerComponent->setBounds({ 0, 0, currentInputsWidth, getHeight() - s_scrollbarsize });
        m_horizontalScrollViewport->setBounds(getLocalBounds().removeFromRight(getWidth() - ctrlsSize));
        m_verticalScrollContainerComponent->setBounds({ 0, 0, ctrlsSize - s_scrollbarsize, currentOutputsHeight });
        m_verticalScrollViewport->setBounds(getLocalBounds().removeFromBottom(getHeight() - ctrlsSize));
    }
    else //if (ControlDirection::None == m_currentIOChannel.first)
    {
        m_horizontalScrollContainerComponent->setBounds({ 0, 0, currentInputsWidth, ctrlsSize - s_scrollbarsize });
        m_horizontalScrollViewport->setBounds(getLocalBounds().removeFromRight(getWidth() - ctrlsSize));
        m_verticalScrollContainerComponent->setBounds({ 0, 0, ctrlsSize - s_scrollbarsize, currentOutputsHeight });
        m_verticalScrollViewport->setBounds(getLocalBounds().removeFromBottom(getHeight() - ctrlsSize));

        m_crosspointsNoSelectionLabel->setBounds(getLocalBounds().removeFromBottom(getHeight() - ctrlsSize).removeFromRight(getWidth() - ctrlsSize));
    }
}

void FaderbankControlComponent::lookAndFeelChanged()
{
    auto ioCount = getIOCount();
    for (auto in = 0; in < ioCount.first; in++)
    {
        if (nullptr != m_inputSelectButtons.at(in))
            m_inputSelectButtons.at(in)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
        if (ControlDirection::Output == m_currentIOChannel.first && m_crosspointGainSliders.size() > in && nullptr != m_crosspointGainSliders.at(in))
            m_crosspointGainSliders.at(in)->setColour(juce::Slider::ColourIds::trackColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    }
    for (auto out = 0; out < ioCount.second; out++)
    {
        if (nullptr != m_outputSelectButtons.at(out))
            m_outputSelectButtons.at(out)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
        if (ControlDirection::Input == m_currentIOChannel.first && m_crosspointGainSliders.size() > out && nullptr != m_crosspointGainSliders.at(out))
            m_crosspointGainSliders.at(out)->setColour(juce::Slider::ColourIds::trackColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    }
}

void FaderbankControlComponent::setControlsSize(const ControlsSize& ctrlsSize)
{
    MemaClientControlComponentBase::setControlsSize(ctrlsSize);
    
    rebuildControls(true);
    selectIOChannel(m_currentIOChannel.first, m_currentIOChannel.second);
}

void FaderbankControlComponent::resetCtrl()
{
    setIOCount({ 0,0 });
    selectIOChannel(ControlDirection::None, 0);
}

void FaderbankControlComponent::setIOCount(const std::pair<int, int>& ioCount)
{
    MemaClientControlComponentBase::setIOCount(ioCount);

    rebuildControls();
    selectIOChannel(m_currentIOChannel.first, m_currentIOChannel.second);
}

void FaderbankControlComponent::rebuildControls(bool force)
{
    rebuildInputControls(force);
    rebuildOutputControls(force);
    rebuildCrosspointControls(force);
    resized();
}

void FaderbankControlComponent::rebuildInputControls(bool force)
{
    auto ioCount = getIOCount();

    if (ioCount.first != m_inputSelectButtons.size() || ioCount.first != m_inputMuteButtons.size() || force)
    {
        auto templateColums = juce::Array<juce::Grid::TrackInfo>();
        for (auto in = 0; in < ioCount.first; in++)
            templateColums.add(juce::Grid::TrackInfo(juce::Grid::Px(m_controlsSize)));

        m_inputMuteButtons.resize(ioCount.first);
        m_inputSelectButtons.resize(ioCount.first);
        m_inputControlsGrid->items.clear();
        m_inputControlsGrid->templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        m_inputControlsGrid->templateColumns = templateColums;

        for (auto i = 0; i < ioCount.first; i++)
        {
            auto in = std::uint16_t(i + 1);
            m_inputSelectButtons.at(i) = std::make_unique<juce::TextButton>("In " + juce::String(in));
            m_inputSelectButtons.at(i)->setClickingTogglesState(true);
            m_inputSelectButtons.at(i)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
            m_inputSelectButtons.at(i)->onClick = [this, i] {
                if (m_inputSelectButtons.size() > i)
                {
                    auto in = std::uint16_t(i + 1);
                    if (m_inputSelectButtons.at(i)->getToggleState())
                        selectIOChannel(ControlDirection::Input, in);
                    else
                        selectIOChannel(ControlDirection::None, 0);
                }
                else
                    jassertfalse;
                };
            m_horizontalScrollContainerComponent->addAndMakeVisible(m_inputSelectButtons.at(i).get());
            m_inputControlsGrid->items.add(juce::GridItem(m_inputSelectButtons.at(i).get()));
        }
        for (auto i = 0; i < ioCount.first; i++)
        {
            auto in = std::uint16_t(i + 1);
            m_inputMuteButtons.at(i) = std::make_unique<juce::TextButton>("M");
            m_inputMuteButtons.at(i)->setClickingTogglesState(true);
            m_inputMuteButtons.at(i)->setToggleState((getInputMuteStates().count(in) != 0 ? getInputMuteStates().at(in) : false), juce::dontSendNotification);
            m_inputMuteButtons.at(i)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
            m_inputMuteButtons.at(i)->onClick = [this, i] {
                auto inputMuteStates = std::map<std::uint16_t, bool>();
                auto in = std::uint16_t(i + 1);
                inputMuteStates[in] = m_inputMuteButtons.at(i)->getToggleState();
                MemaClientControlComponentBase::setInputMuteStates(inputMuteStates);
                if (onInputMutesChanged)
                    onInputMutesChanged(inputMuteStates);
                };
            m_horizontalScrollContainerComponent->addAndMakeVisible(m_inputMuteButtons.at(i).get());
            m_inputControlsGrid->items.add(juce::GridItem(m_inputMuteButtons.at(i).get()));
        }
    }
}

void FaderbankControlComponent::rebuildOutputControls(bool force)
{
    auto ioCount = getIOCount();

    if (ioCount.second != m_outputSelectButtons.size() || ioCount.second != m_outputMuteButtons.size() || force)
    {
        auto templateRows = juce::Array<juce::Grid::TrackInfo>();
        for (auto out = 0; out < ioCount.second; out++)
            templateRows.add(juce::Grid::TrackInfo(juce::Grid::Px(m_controlsSize)));

        m_outputMuteButtons.resize(ioCount.second);
        m_outputSelectButtons.resize(ioCount.second);
        m_outputControlsGrid->items.clear();
        m_outputControlsGrid->templateRows = templateRows;
        m_outputControlsGrid->templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };

        for (auto o = 0; o < ioCount.second; o++)
        {
            auto out = std::uint16_t(o + 1);
            m_outputSelectButtons.at(o) = std::make_unique<juce::TextButton>("Out " + juce::String(out));
            m_outputSelectButtons.at(o)->setClickingTogglesState(true);
            m_outputSelectButtons.at(o)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
            m_outputSelectButtons.at(o)->onClick = [this, o] {
                if (m_outputSelectButtons.size() > o)
                {
                    auto out = std::uint16_t(o + 1);
                    if (m_outputSelectButtons.at(o)->getToggleState())
                        selectIOChannel(ControlDirection::Output, out);
                    else
                        selectIOChannel(ControlDirection::None, 0);
                }
                else
                    jassertfalse;
                };
            m_verticalScrollContainerComponent->addAndMakeVisible(m_outputSelectButtons.at(o).get());
            m_outputControlsGrid->items.add(juce::GridItem(m_outputSelectButtons.at(o).get()));

            m_outputMuteButtons.at(o) = std::make_unique<juce::TextButton>("M");
            m_outputMuteButtons.at(o)->setClickingTogglesState(true);
            m_outputMuteButtons.at(o)->setToggleState((getOutputMuteStates().count(out) != 0 ? getOutputMuteStates().at(out) : false), juce::dontSendNotification);
            m_outputMuteButtons.at(o)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
            m_outputMuteButtons.at(o)->onClick = [this, o] {
                auto outputMuteStates = std::map<std::uint16_t, bool>();
                auto out = std::uint16_t(o + 1);
                outputMuteStates[out] = m_outputMuteButtons.at(o)->getToggleState();
                MemaClientControlComponentBase::setOutputMuteStates(outputMuteStates);
                if (onOutputMutesChanged)
                    onOutputMutesChanged(outputMuteStates);
                };
            m_verticalScrollContainerComponent->addAndMakeVisible(m_outputMuteButtons.at(o).get());
            m_outputControlsGrid->items.add(juce::GridItem(m_outputMuteButtons.at(o).get()));
        }
    }
}

void FaderbankControlComponent::rebuildCrosspointControls(bool force)
{
    auto ioCount = getIOCount();

    if (ControlDirection::Output == m_currentIOChannel.first)
    {
        if (ioCount.first != m_crosspointGainSliders.size() || force)
        {
            auto templateColums = juce::Array<juce::Grid::TrackInfo>();
            for (auto in = 0; in < ioCount.first; in++)
                templateColums.add(juce::Grid::TrackInfo(juce::Grid::Px(m_controlsSize)));

            m_crosspointGainSliders.resize(ioCount.first);
            m_crosspointsControlsGrid->items.clear();
            m_crosspointsControlsGrid->templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
            m_crosspointsControlsGrid->templateColumns = templateColums;

            for (auto i = 0; i < ioCount.first; i++)
            {
                m_crosspointGainSliders.at(i) = std::make_unique<JUCEAppBasics::ToggleStateSlider>(juce::Slider::LinearVertical, juce::Slider::NoTextBox);
                m_crosspointGainSliders.at(i)->setColour(juce::Slider::ColourIds::trackColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
                m_crosspointGainSliders.at(i)->setRange(0.0, 1.0, 0.01);
                m_crosspointGainSliders.at(i)->setTitle(juce::String(i + 1));
                m_crosspointGainSliders.at(i)->displayValueConverter = [](double val) { return juce::String(juce::Decibels::gainToDecibels(val, static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB())), 1) + " dB"; };
                    //juce::Decibels::gainToDecibels(0.0, static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB())),
                    //juce::Decibels::gainToDecibels(1.0, static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB())),
                    //0.1);
                m_crosspointGainSliders.at(i)->onValueChange = [this, i] {
                    auto crosspointValues = std::map<std::uint16_t, std::map<std::uint16_t, float>>();
                    //auto faderValue = juce::Decibels::decibelsToGain(m_crosspointGainSliders.at(i)->getValue(), static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB()));
                    auto faderValue = m_crosspointGainSliders.at(i)->getValue();
                    auto in = std::uint16_t(i + 1);
                    auto out = std::uint16_t(m_currentIOChannel.second);
                    crosspointValues[in][out] = float(faderValue);
                    if (onCrosspointValuesChanged)
                        onCrosspointValuesChanged(crosspointValues);
                    addCrosspointValues(crosspointValues);
                };
                m_crosspointGainSliders.at(i)->onToggleStateChange = [this, i] {
                    auto crosspointStates = std::map<std::uint16_t, std::map<std::uint16_t, bool>>();
                    //auto faderValue = juce::Decibels::decibelsToGain(m_crosspointGainSliders.at(i)->getValue(), static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB()));
                    auto faderState = m_crosspointGainSliders.at(i)->getToggleState();
                    auto in = std::uint16_t(i + 1);
                    auto out = std::uint16_t(m_currentIOChannel.second);
                    crosspointStates[in][out] = faderState;
                    if (onCrosspointStatesChanged)
                        onCrosspointStatesChanged(crosspointStates);
                    addCrosspointStates(crosspointStates);
                    };
                m_horizontalScrollContainerComponent->addAndMakeVisible(m_crosspointGainSliders.at(i).get());
                m_crosspointsControlsGrid->items.add(juce::GridItem(m_crosspointGainSliders.at(i).get()));
            }
        }
        for (auto i = 0; i < ioCount.first; i++)
        {
            if (m_crosspointGainSliders.size() > i)
            {
                auto in = std::uint16_t(i + 1);
                auto out = std::uint16_t(m_currentIOChannel.second);
                auto crosspointState = (getCrosspointStates().count(in) != 0 && getCrosspointStates().at(in).count(out) != 0) ? getCrosspointStates().at(in).at(out) : false;
                auto crosspointValue = (getCrosspointValues().count(in) != 0 && getCrosspointValues().at(in).count(out) != 0) ? getCrosspointValues().at(in).at(out) : 0.0f;
                m_crosspointGainSliders.at(i)->setToggleState(crosspointState, juce::dontSendNotification);
                //m_crosspointGainSliders.at(i)->setValue(juce::Decibels::gainToDecibels(double(crosspointState.second), static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB())), juce::dontSendNotification);
                m_crosspointGainSliders.at(i)->setValue(double(crosspointValue), juce::dontSendNotification);
                m_crosspointGainSliders.at(i)->setVisible(true);
            }
        }
    }
    else if (ControlDirection::Input == m_currentIOChannel.first)
    {
        if (ioCount.second != m_crosspointGainSliders.size() || force)
        {
            auto templateRows = juce::Array<juce::Grid::TrackInfo>();
            for (auto out = 0; out < ioCount.second; out++)
                templateRows.add(juce::Grid::TrackInfo(juce::Grid::Px(m_controlsSize)));

            m_crosspointGainSliders.resize(ioCount.second);
            m_crosspointsControlsGrid->items.clear();
            m_crosspointsControlsGrid->templateColumns = { juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
            m_crosspointsControlsGrid->templateRows = templateRows;

            for (auto o = 0; o < ioCount.second; o++)
            {
                m_crosspointGainSliders.at(o) = std::make_unique<JUCEAppBasics::ToggleStateSlider>(juce::Slider::LinearHorizontal, juce::Slider::NoTextBox);
                m_crosspointGainSliders.at(o)->setColour(juce::Slider::ColourIds::trackColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
                m_crosspointGainSliders.at(o)->setRange(0.0, 1.0, 0.01);
                m_crosspointGainSliders.at(o)->setTitle(juce::String(o + 1));
                m_crosspointGainSliders.at(o)->displayValueConverter = [](double val) { return juce::String(juce::Decibels::gainToDecibels(val, static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB())), 1) + " dB"; };
                    //juce::Decibels::gainToDecibels(0.0, static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB())),
                    //juce::Decibels::gainToDecibels(1.0, static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB())),
                    //0.1);
                m_crosspointGainSliders.at(o)->onValueChange = [this, o] {
                    auto crosspointValues = std::map<std::uint16_t, std::map<std::uint16_t, float>>();
                    //auto faderValue = juce::Decibels::decibelsToGain(m_crosspointGainSliders.at(o)->getValue(), static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB()));
                    auto faderValue = m_crosspointGainSliders.at(o)->getValue();
                    auto in = std::uint16_t(m_currentIOChannel.second);
                    auto out = std::uint16_t(o + 1);
                    crosspointValues[in][out] = float(faderValue);
                    if (onCrosspointValuesChanged)
                        onCrosspointValuesChanged(crosspointValues);
                    addCrosspointValues(crosspointValues);
                };
                m_crosspointGainSliders.at(o)->onToggleStateChange = [this, o] {
                    auto crosspointStates = std::map<std::uint16_t, std::map<std::uint16_t, bool>>();
                    auto faderState = m_crosspointGainSliders.at(o)->getToggleState();
                    auto in = std::uint16_t(m_currentIOChannel.second);
                    auto out = std::uint16_t(o + 1);
                    crosspointStates[in][out] = faderState;
                    if (onCrosspointStatesChanged)
                        onCrosspointStatesChanged(crosspointStates);
                    addCrosspointStates(crosspointStates);
                    };
                m_verticalScrollContainerComponent->addAndMakeVisible(m_crosspointGainSliders.at(o).get());
                m_crosspointsControlsGrid->items.add(juce::GridItem(m_crosspointGainSliders.at(o).get()));

            }
        }
        for (auto o = 0; o < ioCount.second; o++)
        {
            if (m_crosspointGainSliders.size() > o)
            {
                auto in = std::uint16_t(m_currentIOChannel.second);
                auto out = std::uint16_t(o + 1);
                auto crosspointState = ((getCrosspointStates().count(in) != 0) != 0 && getCrosspointStates().at(in).count(out) != 0) ? getCrosspointStates().at(in).at(out) : false;
                auto crosspointValue = ((getCrosspointValues().count(in) != 0) != 0 && getCrosspointValues().at(in).count(out) != 0) ? getCrosspointValues().at(in).at(out) : 0.0f;
                m_crosspointGainSliders.at(o)->setToggleState(crosspointState, juce::dontSendNotification);
                //m_crosspointGainSliders.at(o)->setValue(juce::Decibels::gainToDecibels(double(crosspointState.second), static_cast<double>(ProcessorDataAnalyzer::getGlobalMindB())), juce::dontSendNotification);
                m_crosspointGainSliders.at(o)->setValue(double(crosspointValue), juce::dontSendNotification);
                m_crosspointGainSliders.at(o)->setVisible(true);
            }
        }
    }
}

void FaderbankControlComponent::setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates)
{
    MemaClientControlComponentBase::setInputMuteStates(inputMuteStates);

    for (auto const& inputMuteStateKV : inputMuteStates)
    {
        auto& in = inputMuteStateKV.first;
        auto i = in - 1;
        auto& state = inputMuteStateKV.second;
        if (m_inputMuteButtons.size() > i && nullptr != m_inputMuteButtons.at(i))
            m_inputMuteButtons.at(i)->setToggleState(state, juce::dontSendNotification);
    }
}

void FaderbankControlComponent::setOutputMuteStates(const std::map<std::uint16_t, bool>& outputMuteStates)
{
    MemaClientControlComponentBase::setOutputMuteStates(outputMuteStates);

    for (auto const& outputMuteStateKV : outputMuteStates)
    {
        auto& out = outputMuteStateKV.first;
        auto o = out - 1;
        auto& state = outputMuteStateKV.second;
        if (m_outputMuteButtons.size() > o && nullptr != m_outputMuteButtons.at(o))
            m_outputMuteButtons.at(o)->setToggleState(state, juce::dontSendNotification);
    }
}

void FaderbankControlComponent::setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates)
{
    MemaClientControlComponentBase::setCrosspointStates(crosspointStates);

    updateCrosspointFaderValues();
}

void FaderbankControlComponent::setCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues)
{
    MemaClientControlComponentBase::setCrosspointValues(crosspointValues);

    updateCrosspointFaderValues();
}

void FaderbankControlComponent::selectIOChannel(const ControlDirection& direction, int channel)
{
    jassert((direction == ControlDirection::None && channel == 0) || (direction != ControlDirection::None && channel != 0));
    auto oldDirection = m_currentIOChannel.first;
    auto oldChannel = m_currentIOChannel.second;
    m_currentIOChannel = std::make_pair(direction, channel);
    if (oldDirection != direction)
        rebuildControls();
    else if (oldChannel != channel)
        rebuildControls();

    auto ioCount = getIOCount();
    for (auto i = 0; i < ioCount.first; i++)
    {
        if (nullptr != m_inputSelectButtons.at(i))
        {
            auto in = std::uint16_t(i + 1);
            auto state = (ControlDirection::Input == direction && channel == in);
            m_inputSelectButtons.at(i)->setToggleState(state, juce::dontSendNotification);
        }
    }
    for (auto o = 0; o < ioCount.second; o++)
    {
        if (nullptr != m_outputSelectButtons.at(o))
        {
            auto out = std::uint16_t(o + 1);
            auto state = (ControlDirection::Output == direction && channel == out);
            m_outputSelectButtons.at(o)->setToggleState(state, juce::dontSendNotification);
        }
    }
}

void FaderbankControlComponent::updateCrosspointFaderValues()
{
    auto& crosspointStates = getCrosspointStates();
    auto crosspointValues = getCrosspointValues();
    for (auto const& crosspointStateIKV : crosspointStates)
    {
        auto& in = crosspointStateIKV.first;
        for (auto const& crosspointStateIOKV : crosspointStateIKV.second)
        {
            auto& out = crosspointStateIOKV.first;
            auto& state = crosspointStateIOKV.second;
            auto& value = crosspointValues[in][out];

            auto i = in - 1;
            auto o = out - 1;

            if (ControlDirection::Input == m_currentIOChannel.first && in == m_currentIOChannel.second && m_crosspointGainSliders.size() > o)
            {
                m_crosspointGainSliders.at(o)->setToggleState(state, juce::dontSendNotification);
                m_crosspointGainSliders.at(o)->setValue(value, juce::dontSendNotification);
            }
            if (ControlDirection::Output == m_currentIOChannel.first && out == m_currentIOChannel.second && m_crosspointGainSliders.size() > i)
            {
                m_crosspointGainSliders.at(i)->setToggleState(state, juce::dontSendNotification);
                m_crosspointGainSliders.at(i)->setValue(value, juce::dontSendNotification);
            }
        }
    }
}


PanningControlComponent::PanningControlComponent()
    : MemaClientControlComponentBase()
{
    m_multiSlider = std::make_unique<Mema::TwoDFieldMultisliderComponent>();
    m_multiSlider->onInputPositionChanged = [=](std::uint16_t channel, const Mema::TwoDFieldMultisliderComponent::TwoDMultisliderValue& value, std::optional<Mema::TwoDFieldMultisliderComponent::ChannelLayer> layer) {
        changeInputPosition(channel, value.relXPos, value.relYPos, layer.has_value() ? layer.value() : 0);
    };
    m_multiSlider->onInputToOutputStatesChanged = [=](const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& inputToOutputStates) {
        addCrosspointStates(inputToOutputStates);
        if (onCrosspointStatesChanged)
            onCrosspointStatesChanged(getCrosspointStates());
    };
    m_multiSlider->onInputToOutputValuesChanged = [=](const std::map<std::uint16_t, std::map<std::uint16_t, float>>& inputToOutputValues) {
        addCrosspointValues(inputToOutputValues);
        if (onCrosspointValuesChanged)
            onCrosspointValuesChanged(getCrosspointValues());
    };
    m_multiSlider->onInputSelected = [=](std::uint16_t channel) {
        selectInputChannel(channel);
    };
    addAndMakeVisible(m_multiSlider.get());

    m_sharpnessEdit = std::make_unique<juce::TextEditor>("SharpnessEdit");
    m_sharpnessEdit->setTooltip("Panning sharpness 0.0 ... 1.0");
    m_sharpnessEdit->setText(juce::String(m_panningSharpness), false);
    m_sharpnessEdit->setInputFilter(new juce::TextEditor::LengthAndCharacterRestriction(3, "0123456789."), true);
    m_sharpnessEdit->onReturnKey = [=]() { setPanningSharpness(m_sharpnessEdit->getText().getFloatValue()); };
    addAndMakeVisible(m_sharpnessEdit.get());
    m_sharpnessLabel = std::make_unique<juce::Label>("Sharpness");
    m_sharpnessLabel->setText("Sharpness", juce::dontSendNotification);
    m_sharpnessLabel->attachToComponent(m_sharpnessEdit.get(), true);

    m_horizontalScrollContainerComponent = std::make_unique<juce::Component>();
    m_horizontalScrollViewport = std::make_unique<juce::Viewport>();
    m_horizontalScrollViewport->setViewedComponent(m_horizontalScrollContainerComponent.get(), false);
    addAndMakeVisible(m_horizontalScrollViewport.get());

    m_inputControlsGrid = std::make_unique<juce::Grid>();
    m_inputControlsGrid->setGap(juce::Grid::Px(s_gap));
}

PanningControlComponent::~PanningControlComponent()
{
}

void PanningControlComponent::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    auto ctrlsSize = 2 * (m_controlsSize + s_gap);
    g.setColour(getLookAndFeel().findColour(juce::Slider::backgroundColourId));
    g.fillRect(getLocalBounds().removeFromTop(ctrlsSize).removeFromRight(getWidth() - ctrlsSize));
    g.fillRect(getLocalBounds().removeFromTop(ctrlsSize + s_scrollbarsize).removeFromBottom(s_scrollbarsize));
}

void PanningControlComponent::resized()
{
    auto margin = 8;
    auto bounds = getLocalBounds().reduced(margin, margin);

    auto ctrlsSize = 2 * (m_controlsSize + s_gap) + s_scrollbarsize;
    auto currentInputsWidth = (m_inputControlsGrid->getNumberOfColumns() * (m_controlsSize + s_gap)) - s_gap;

    if (m_inputControlsGrid)
        m_inputControlsGrid->performLayout({ 0, 0, currentInputsWidth, ctrlsSize - s_scrollbarsize });

    m_horizontalScrollContainerComponent->setBounds({ 0, 0, currentInputsWidth, ctrlsSize - s_scrollbarsize });
    m_horizontalScrollViewport->setBounds(getLocalBounds().removeFromTop(ctrlsSize).removeFromRight(getWidth() - ctrlsSize));

    bounds.removeFromTop(ctrlsSize);

    auto boundsAspect = bounds.toFloat().getAspectRatio();
    auto fieldAspect = m_multiSlider->getRequiredAspectRatio();
    if (boundsAspect >= 1 / fieldAspect)
    {
        // landscape
        auto multiSliderBounds = juce::Rectangle<int>(int(bounds.getHeight() / fieldAspect), bounds.getHeight()).withCentre(bounds.getCentre());
        if (m_multiSlider)
            m_multiSlider->setBounds(multiSliderBounds);
        if (m_sharpnessEdit)
            m_sharpnessEdit->setBounds(multiSliderBounds.removeFromBottom(20).removeFromLeft(50));
    }
    else
    {
        // portrait
        auto multiSliderBounds = juce::Rectangle<int>(bounds.getWidth(), int(bounds.getWidth() * fieldAspect)).withCentre(bounds.getCentre());
        if (m_multiSlider)
            m_multiSlider->setBounds(multiSliderBounds);
        if (m_sharpnessEdit)
            m_sharpnessEdit->setBounds(multiSliderBounds.removeFromBottom(20).removeFromLeft(50));
    }
}

void PanningControlComponent::lookAndFeelChanged()
{
    auto ioCount = getIOCount();
    for (auto in = 0; in < ioCount.first; in++)
    {
        if (nullptr != m_inputSelectButtons.at(in))
            m_inputSelectButtons.at(in)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
    }
}

void PanningControlComponent::setControlsSize(const ControlsSize& ctrlsSize)
{
    MemaClientControlComponentBase::setControlsSize(ctrlsSize);

    rebuildControls(true);
    selectInputChannel(m_currentInputChannel);
}

void PanningControlComponent::resetCtrl()
{
    setIOCount({ 0,0 });
    selectInputChannel(0);
}

void PanningControlComponent::setIOCount(const std::pair<int, int>& ioCount)
{
    MemaClientControlComponentBase::setIOCount(ioCount);

    rebuildControls();

    if (m_multiSlider)
        m_multiSlider->setIOCount(ioCount);

    selectInputChannel(m_currentInputChannel);
}

void PanningControlComponent::setChannelConfig(const juce::AudioChannelSet& channelConfiguration)
{
    m_channelConfiguration = channelConfiguration;

    if (m_multiSlider)
        m_multiSlider->setChannelConfiguration(channelConfiguration);
}

const juce::AudioChannelSet& PanningControlComponent::getChannelConfig()
{
    return m_channelConfiguration;
}

void PanningControlComponent::rebuildControls(bool force)
{
    rebuildInputControls(force);
    resized();
}

void PanningControlComponent::rebuildInputControls(bool force)
{
    auto ioCount = getIOCount();

    if (ioCount.first != m_inputSelectButtons.size() || ioCount.first != m_inputMuteButtons.size() || force)
    {
        auto templateColums = juce::Array<juce::Grid::TrackInfo>();
        for (auto in = 0; in < ioCount.first; in++)
            templateColums.add(juce::Grid::TrackInfo(juce::Grid::Px(m_controlsSize)));

        m_inputMuteButtons.resize(ioCount.first);
        m_inputSelectButtons.resize(ioCount.first);
        m_inputControlsGrid->items.clear();
        m_inputControlsGrid->templateRows = { juce::Grid::TrackInfo(juce::Grid::Fr(1)), juce::Grid::TrackInfo(juce::Grid::Fr(1)) };
        m_inputControlsGrid->templateColumns = templateColums;

        for (auto i = 0; i < ioCount.first; i++)
        {
            auto in = std::uint16_t(i + 1);
            m_inputSelectButtons.at(i) = std::make_unique<juce::TextButton>("In " + juce::String(in));
            m_inputSelectButtons.at(i)->setClickingTogglesState(true);
            m_inputSelectButtons.at(i)->setColour(juce::TextButton::ColourIds::buttonOnColourId, getLookAndFeel().findColour(JUCEAppBasics::CustomLookAndFeel::ColourIds::MeteringRmsColourId));
            m_inputSelectButtons.at(i)->onClick = [this, i] {
                if (m_inputSelectButtons.size() > i)
                {
                    auto in = std::uint16_t(i + 1);
                    if (m_inputSelectButtons.at(i)->getToggleState())
                        selectInputChannel(in);
                    else
                        selectInputChannel(0);
                }
                else
                    jassertfalse;
                };
            m_horizontalScrollContainerComponent->addAndMakeVisible(m_inputSelectButtons.at(i).get());
            m_inputControlsGrid->items.add(juce::GridItem(m_inputSelectButtons.at(i).get()));
        }
        for (auto i = 0; i < ioCount.first; i++)
        {
            auto in = std::uint16_t(i + 1);
            m_inputMuteButtons.at(i) = std::make_unique<juce::TextButton>("M");
            m_inputMuteButtons.at(i)->setClickingTogglesState(true);
            m_inputMuteButtons.at(i)->setToggleState((getInputMuteStates().count(in) != 0 ? getInputMuteStates().at(in) : false), juce::dontSendNotification);
            m_inputMuteButtons.at(i)->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
            m_inputMuteButtons.at(i)->onClick = [this, i] {
                auto inputMuteStates = std::map<std::uint16_t, bool>();
                auto in = std::uint16_t(i + 1);
                inputMuteStates[in] = m_inputMuteButtons.at(i)->getToggleState();
                MemaClientControlComponentBase::setInputMuteStates(inputMuteStates);
                if (onInputMutesChanged)
                    onInputMutesChanged(inputMuteStates);
                };
            m_horizontalScrollContainerComponent->addAndMakeVisible(m_inputMuteButtons.at(i).get());
            m_inputControlsGrid->items.add(juce::GridItem(m_inputMuteButtons.at(i).get()));
        }
    }
}

void PanningControlComponent::setInputMuteStates(const std::map<std::uint16_t, bool>& inputMuteStates)
{
    MemaClientControlComponentBase::setInputMuteStates(inputMuteStates);

    for (auto const& inputMuteStateKV : inputMuteStates)
    {
        auto& in = inputMuteStateKV.first;
        auto i = in - 1;
        auto& state = inputMuteStateKV.second;
        if (m_inputMuteButtons.size() > i && nullptr != m_inputMuteButtons.at(i))
            m_inputMuteButtons.at(i)->setToggleState(state, juce::dontSendNotification);
    }
}

void PanningControlComponent::setCrosspointStates(const std::map<std::uint16_t, std::map<std::uint16_t, bool>>& crosspointStates)
{
    MemaClientControlComponentBase::setCrosspointStates(crosspointStates);

    if (m_multiSlider)
        m_multiSlider->setInputToOutputStates(crosspointStates);
}

void PanningControlComponent::setCrosspointValues(const std::map<std::uint16_t, std::map<std::uint16_t, float>>& crosspointValues)
{
    MemaClientControlComponentBase::setCrosspointValues(crosspointValues);

    if (m_multiSlider)
        m_multiSlider->setInputToOutputLevels(crosspointValues);
}

void PanningControlComponent::selectInputChannel(std::uint16_t channel)
{
    auto oldChannel = m_currentInputChannel;
    m_currentInputChannel = channel;
    if (oldChannel != channel)
        rebuildControls();

    auto ioCount = getIOCount();
    for (auto i = 0; i < ioCount.first; i++)
    {
        auto in = std::uint16_t(i + 1);
        auto state = channel == in;
        if (nullptr != m_inputSelectButtons.at(i))
            m_inputSelectButtons.at(i)->setToggleState(state, juce::dontSendNotification);
        if (nullptr != m_multiSlider)
            m_multiSlider->selectInput(in, state);
    }
}

void PanningControlComponent::changeInputPosition(std::uint16_t channel, float xVal, float yVal, int layer)
{
    //DBG(juce::String(__FUNCTION__) << " new pos: " << int(channel) << " " << xVal << "," << yVal << "(" << layer << ")");

    if (m_multiSlider)
    {
        std::map<juce::AudioChannelSet::ChannelType, juce::Point<float>> outputsMaxPoints;
        std::map<juce::AudioChannelSet::ChannelType, float> channelToOutputsDists;

        auto c = juce::Point<float>(0.0f, 0.0f);

        auto outputs = m_multiSlider->getOutputsInLayer(TwoDFieldMultisliderComponent::ChannelLayer(layer));
        for (auto const& channelType : outputs)
        {
            auto angleRad = juce::degreesToRadians(m_multiSlider->getAngleForChannelTypeInCurrentConfiguration(channelType));
            auto xLength = sinf(angleRad);
            auto yLength = cosf(angleRad);
            outputsMaxPoints[channelType] = juce::Point<float>(xLength, -yLength);

            // this is the actual primitive sourceposition-to-output level calculation algorithm
            auto inputPosition = juce::Point<float>(xVal, yVal);
            auto outputMaxPoint = outputsMaxPoints[channelType];
            auto distance = outputMaxPoint.getDistanceFrom(inputPosition);
            auto base = 0.5f * distance;
            auto exp = jmap(m_panningSharpness, 1.0f, 5.0f);
            channelToOutputsDists[channelType] = powf(base, exp);

            //DBG(juce::String(__FUNCTION__) << " " << juce::AudioChannelSet::getAbbreviatedChannelTypeName(channelType) << ": " << channelToOutputsDists[channelType]);
        }
        auto outputsNotInLayer = m_multiSlider->getDirectiveOutputsNotInLayer(TwoDFieldMultisliderComponent::ChannelLayer(layer));
        for (auto const& channelType : outputsNotInLayer)
        {
            channelToOutputsDists[channelType] = 0.0f;
        }

        std::map<std::uint16_t, std::map<std::uint16_t, bool>> crosspointStates;
        std::map<std::uint16_t, std::map<std::uint16_t, float>> crosspointValues;
        for (auto const& cToOdKV : channelToOutputsDists)
        {
            crosspointStates[channel][std::uint16_t(m_multiSlider->getChannelNumberForChannelTypeInCurrentConfiguration(cToOdKV.first))] = true;
            crosspointValues[channel][std::uint16_t(m_multiSlider->getChannelNumberForChannelTypeInCurrentConfiguration(cToOdKV.first))] = cToOdKV.second;
        }
        if (onCrosspointStatesChanged)
            onCrosspointStatesChanged(crosspointStates);
        addCrosspointStates(crosspointStates);
        if (onCrosspointValuesChanged)
            onCrosspointValuesChanged(crosspointValues);
        addCrosspointValues(crosspointValues);
    }
}

void PanningControlComponent::setPanningSharpness(float sharpness)
{
    m_panningSharpness = jlimit(0.0f, 1.0f, sharpness);

    if (m_panningSharpness != sharpness && m_sharpnessEdit)
        m_sharpnessEdit->setText(juce::String(m_panningSharpness));
    else if (m_multiSlider)
        m_multiSlider->triggerInputPositionsDump();
}


} // namespace Mema

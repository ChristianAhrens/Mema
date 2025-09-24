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

#include "PanningControlComponent.h"

#include <CustomLookAndFeel.h>
#include <ToggleStateSlider.h>
#include <MemaClientCommon/ADMOSController.h>
#include <MemaClientCommon/InputPositionMapper.h>
#include <MemaClientCommon/TwoDFieldMultisliderComponent.h>


namespace Mema
{


PanningControlComponent::PanningControlComponent()
    : MemaClientControlComponentBase()
{
    m_multiSlider = std::make_unique<Mema::TwoDFieldMultisliderComponent>();
    m_multiSlider->onInputPositionChanged = [=](std::uint16_t channel, const Mema::TwoDFieldMultisliderComponent::TwoDMultisliderValue& value, const float& sharpness, std::optional<Mema::TwoDFieldMultisliderComponent::ChannelLayer> layer) {
        changeInputPosition(channel, value.relXPos, value.relYPos, sharpness, layer.has_value() ? layer.value() : 0);
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

    m_positionMapper = std::make_unique<InputPositionMapper>();
    m_positionMapper->onInputPositionMapped = [=](std::uint16_t channel, const std::map<juce::AudioChannelSet::ChannelType, float>& channelToOutputsDists) {
        processOutputDistances(channel, channelToOutputsDists);
    };
    m_positionMapper->getAngleForChannelType = [=](juce::AudioChannelSet::ChannelType channelType) {
        if (m_multiSlider)
            return m_multiSlider->getAngleForChannelTypeInCurrentConfiguration(channelType);
        else
            return 0.0f;
    };

    m_admOsController = std::make_unique<ADMOSController>();
    m_admOsController->onParameterChanged = [=](int objNum, std::uint16_t objType) {
        handleExternalControlParameter(objNum, objType, m_admOsController.get());
    };

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
    auto bounds = getLocalBounds();
    g.fillRect(bounds.removeFromTop(ctrlsSize).removeFromRight(getWidth() - ctrlsSize));
    g.fillRect(bounds);
}

void PanningControlComponent::resized()
{
    auto ctrlsSize = 2 * (m_controlsSize + s_gap) + s_scrollbarsize;
    auto currentInputsWidth = (m_inputControlsGrid->getNumberOfColumns() * (m_controlsSize + s_gap)) - s_gap;

    if (m_inputControlsGrid)
        m_inputControlsGrid->performLayout({ 0, 0, currentInputsWidth, ctrlsSize - s_scrollbarsize });

    m_horizontalScrollContainerComponent->setBounds({ 0, 0, currentInputsWidth, ctrlsSize - s_scrollbarsize });
    m_horizontalScrollViewport->setBounds(getLocalBounds().removeFromTop(ctrlsSize).removeFromRight(getWidth() - ctrlsSize));

    auto panningBounds = getLocalBounds();
    panningBounds.removeFromTop(ctrlsSize);

    auto boundsAspect = panningBounds.toFloat().getAspectRatio();
    auto fieldAspect = m_multiSlider->getRequiredAspectRatio();
    if (boundsAspect >= 1 / fieldAspect)
    {
        // landscape
        auto multiSliderBounds = juce::Rectangle<int>(int(panningBounds.getHeight() / fieldAspect), panningBounds.getHeight()).withCentre(panningBounds.getCentre());
        if (m_multiSlider)
            m_multiSlider->setBounds(multiSliderBounds);
    }
    else
    {
        // portrait
        auto multiSliderBounds = juce::Rectangle<int>(panningBounds.getWidth(), int(panningBounds.getWidth() * fieldAspect)).withCentre(panningBounds.getCentre());
        if (m_multiSlider)
            m_multiSlider->setBounds(multiSliderBounds);
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

    if (m_multiSlider)
        m_multiSlider->setControlsSize(ctrlsSize);
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
    if (m_admOsController)
        m_admOsController->setNumObjects(ioCount.first);

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

void PanningControlComponent::setExternalControlSettings(int ADMOSCPort, const juce::IPAddress& ADMOSCControllerIP, int ADMOSCControllerPort)
{
    if (m_admOsController)
        m_admOsController->startConnection(ADMOSCPort, ADMOSCControllerIP, ADMOSCControllerPort);
}

void PanningControlComponent::handleExternalControlParameter(int objNum, std::uint16_t objType, void* /*sender*/)
{
    auto admObjType = static_cast<ADMOSController::ADMOSCParameterType>(objType);
    DBG(juce::String(__FUNCTION__) << " handle n" << objNum << " t" << admObjType);

    jassert(m_admOsController);
    if (!m_admOsController)
        return;
    jassert(m_multiSlider);
    if (!m_multiSlider)
        return;

    switch (admObjType)
    {
    case ADMOSController::ADMOSCParameterType::X:
        {
            auto xParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::X);
            auto xVal = ADMOSController::ADMOSCParameterX(xParam).getParameterVal();

            auto xyzParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::XYZ);
            auto xyzVals = ADMOSController::ADMOSCParameterXYZ(xyzParam).getParameterVals();
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterXY(xVal, std::get<1>(xyzVals))); // fill up with y from xyzParam
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterXYZ(xVal, std::get<1>(xyzVals), std::get<2>(xyzVals))); // fill up with y+z from xyzParam

            changeInputPosition(std::uint16_t(objNum), xVal, std::nullopt, std::nullopt, std::nullopt, juce::NotificationType::sendNotification);
        }
        break;
    case ADMOSController::ADMOSCParameterType::Y:
        {
            auto yParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::Y);
            auto yVal = ADMOSController::ADMOSCParameterY(yParam).getParameterVal();

            auto xyzParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::XYZ);
            auto xyzVals = ADMOSController::ADMOSCParameterXYZ(xyzParam).getParameterVals();
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterXY(std::get<0>(xyzVals), yVal)); // fill up with y from xyzParam
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterXYZ(std::get<0>(xyzVals), yVal, std::get<2>(xyzVals))); // fill up with y+z from xyzParam

            changeInputPosition(std::uint16_t(objNum), std::nullopt, yVal, std::nullopt, std::nullopt, juce::NotificationType::sendNotification);
        }
        break;
    case ADMOSController::ADMOSCParameterType::Z:
        {
            auto zParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::Z);
            auto zVal = ADMOSController::ADMOSCParameterZ(zParam).getParameterVal();

            auto xyzParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::XYZ);
            auto xyzVals = ADMOSController::ADMOSCParameterXYZ(xyzParam).getParameterVals();
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterXYZ(std::get<1>(xyzVals), std::get<1>(xyzVals), zVal)); // fill up with y+z from xyzParam

            changeInputPosition(std::uint16_t(objNum), std::nullopt, std::nullopt, std::nullopt, (zVal > 0.5f ? TwoDFieldMultisliderComponent::ChannelLayer::PositionedHeight : TwoDFieldMultisliderComponent::ChannelLayer::Positioned), juce::NotificationType::sendNotification);
        }
        break;
    case ADMOSController::ADMOSCParameterType::XY:
        {
            auto xyParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::XY);
            auto xyVals = ADMOSController::ADMOSCParameterXY(xyParam).getParameterVals();

            auto xyzParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::XYZ);
            auto xyzVals = ADMOSController::ADMOSCParameterXYZ(xyzParam).getParameterVals();
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterX(std::get<0>(xyVals)));
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterY(std::get<1>(xyVals)));
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterXYZ(std::get<0>(xyVals), std::get<1>(xyVals), std::get<2>(xyzVals))); // fill up with z from xyzParam

            changeInputPosition(std::uint16_t(objNum), std::get<0>(xyVals), std::get<1>(xyVals), std::nullopt, std::nullopt, juce::NotificationType::sendNotification);
        }
        break;
    case ADMOSController::ADMOSCParameterType::XYZ:
        {
            auto xyzParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::XYZ);
            auto xyzVals = ADMOSController::ADMOSCParameterXYZ(xyzParam).getParameterVals();

            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterX(std::get<0>(xyzVals)));
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterY(std::get<1>(xyzVals)));
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterZ(std::get<2>(xyzVals)));
            m_admOsController->setParameter(objNum, ADMOSController::ADMOSCParameterXY(std::get<0>(xyzVals), std::get<1>(xyzVals)));

            changeInputPosition(std::uint16_t(objNum), std::get<0>(xyzVals), std::get<1>(xyzVals), std::nullopt, (std::get<2>(xyzVals) > 0.5f ? TwoDFieldMultisliderComponent::ChannelLayer::PositionedHeight : TwoDFieldMultisliderComponent::ChannelLayer::Positioned), juce::NotificationType::sendNotification);
        }
        break;
    case ADMOSController::ADMOSCParameterType::Width:
        {
            auto wParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::Width);
            auto widthVal = ADMOSController::ADMOSCParameterWidth(wParam).getParameterVal();

            changeInputPosition(std::uint16_t(objNum), std::nullopt, std::nullopt, 1.0f - widthVal, std::nullopt, juce::NotificationType::sendNotification);
        }
        break;
    case ADMOSController::ADMOSCParameterType::Mute:
        {
            auto mParam = m_admOsController->getParameter(objNum, ADMOSController::ADMOSCParameterType::Mute);
            auto muteVal = ADMOSController::ADMOSCParameterMute(mParam).getParameterVal();

            setInputMuteStates({ std::make_pair(static_cast<std::uint16_t>(objNum), muteVal) });
        }
        break;
    case ADMOSController::ADMOSCParameterType::Empty:
    default:
        break;
    }
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
                if (m_admOsController)
                    m_admOsController->setParameter(in, ADMOSController::ADMOSCParameterMute(inputMuteStates[in]), ADMOSController::ADMOSCParameterChangeTarget::External);
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

void PanningControlComponent::changeInputPosition(std::uint16_t channel, std::optional<float> xValOpt, std::optional<float> yValOpt, std::optional<float> sharpnessOpt, std::optional<int> layerOpt, juce::NotificationType notification)
{
    float xVal = 0.0f;
    if (xValOpt.has_value())
        xVal = xValOpt.value();
    else if (m_admOsController)
        xVal = ADMOSController::ADMOSCParameterX(m_admOsController->getParameter(channel, ADMOSController::ADMOSCParameterType::X)).getParameterVal();

    float yVal = 0.0f;
    if (yValOpt.has_value())
        yVal = yValOpt.value();
    else if (m_admOsController)
        yVal = ADMOSController::ADMOSCParameterY(m_admOsController->getParameter(channel, ADMOSController::ADMOSCParameterType::Y)).getParameterVal();

    float sharpnessVal = 0.0f;
    if (sharpnessOpt.has_value())
        sharpnessVal = sharpnessOpt.value();
    else if (m_admOsController)
        sharpnessVal = 1.0f - ADMOSController::ADMOSCParameterWidth(m_admOsController->getParameter(channel, ADMOSController::ADMOSCParameterType::Width)).getParameterVal();

    int layerVal = 0;
    if (layerOpt.has_value())
        layerVal = layerOpt.value();
    else if (m_admOsController)
    {
        auto zVal = ADMOSController::ADMOSCParameterZ(m_admOsController->getParameter(channel, ADMOSController::ADMOSCParameterType::Z)).getParameterVal();
        layerVal = zVal > 0.5f ? TwoDFieldMultisliderComponent::ChannelLayer::PositionedHeight : TwoDFieldMultisliderComponent::ChannelLayer::Positioned;
    }

    DBG(juce::String(__FUNCTION__) << " new pos: " << int(channel) << " " << xVal << "," << yVal << "(" << sharpnessVal << " / " << layerVal << ")");

    if (m_multiSlider && m_positionMapper)
    {
        m_positionMapper->setOutputIncludePositions(m_multiSlider->getOutputsInLayer(TwoDFieldMultisliderComponent::ChannelLayer(layerVal)));
        m_positionMapper->setOutputIgnorePositions(m_multiSlider->getDirectiveOutputsNotInLayer(TwoDFieldMultisliderComponent::ChannelLayer(layerVal)));
        m_positionMapper->mapInputPosition(channel, { -xVal, yVal }, sharpnessVal);

        if (juce::NotificationType::dontSendNotification != notification)
        {
            if (xValOpt.has_value() || yValOpt.has_value())
                m_multiSlider->setInputPositionValue(channel, TwoDFieldMultisliderComponent::TwoDMultisliderValue(xVal, yVal), juce::dontSendNotification);
            if (layerOpt.has_value())
                m_multiSlider->setInputPositionLayer(channel, TwoDFieldMultisliderComponent::ChannelLayer(layerVal), juce::dontSendNotification);
            if (sharpnessOpt.has_value())
                m_multiSlider->setInputPositionSharpness(channel, sharpnessVal, juce::dontSendNotification);
        }
    }

    if (m_admOsController)
    {
        auto admTarget = juce::NotificationType::dontSendNotification == notification ? ADMOSController::ADMOSCParameterChangeTarget::External : ADMOSController::ADMOSCParameterChangeTarget::None;
        if (xValOpt.has_value() || yValOpt.has_value())
            m_admOsController->setParameter(channel, ADMOSController::ADMOSCParameterXY(xVal, yVal), admTarget);
        if (xValOpt.has_value())
            m_admOsController->setParameter(channel, ADMOSController::ADMOSCParameterX(xVal), ADMOSController::ADMOSCParameterChangeTarget::None);
        if (yValOpt.has_value())
            m_admOsController->setParameter(channel, ADMOSController::ADMOSCParameterY(yVal), ADMOSController::ADMOSCParameterChangeTarget::None);
        if (xValOpt.has_value() || yValOpt.has_value() || layerOpt.has_value())
            m_admOsController->setParameter(channel, ADMOSController::ADMOSCParameterXYZ(xVal, yVal, TwoDFieldMultisliderComponent::ChannelLayer::PositionedHeight == layerVal ? 1.0f : 0.0f), ADMOSController::ADMOSCParameterChangeTarget::None);
        if (sharpnessOpt.has_value())
            m_admOsController->setParameter(channel, ADMOSController::ADMOSCParameterWidth(1.0f - sharpnessVal), admTarget);
    }
}

void PanningControlComponent::processOutputDistances(std::uint16_t channel, const std::map<juce::AudioChannelSet::ChannelType, float>& channelToOutputsDists)
{
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


} // namespace Mema

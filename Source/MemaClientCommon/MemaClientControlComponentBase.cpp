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

#include "MemaClientControlComponentBase.h"

#include <CustomLookAndFeel.h>


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


} // namespace Mema

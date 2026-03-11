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
#pragma once

#include <JuceHeader.h>

namespace Mema
{

enum class ParameterControlType
{
    Toggle,      // Two-state button/switch
    Discrete,    // Dropdown/ComboBox for discrete values
    Continuous   // Slider/Fader for continuous values
};

/** @brief Metadata describing a single plugin parameter exposed for remote control. */
struct PluginParameterInfo
{
    int index = 0;                              ///< Zero-based parameter index within the plugin.
    juce::String id = "0";                      ///< Unique string identifier of the parameter.
    juce::String name = "";                     ///< Human-readable parameter name.
    float defaultValue = 0.0f;                  ///< Factory default value (normalised 0..1).
    float currentValue = 0.0f;                  ///< Current parameter value (normalised 0..1).
    juce::String label = "";                    ///< Unit label (e.g. "dB", "Hz").
    bool isAutomatable = false;                 ///< Whether the host can automate this parameter.
    bool isRemoteControllable = true;           ///< Whether this parameter is exposed for remote control.
    juce::AudioProcessorParameter::Category category = juce::AudioProcessorParameter::Category::genericParameter; ///< JUCE parameter category.
    float minValue = 0.0f;                      ///< Minimum value in the parameter's native range.
    float maxValue = 1.0f;                      ///< Maximum value in the parameter's native range.
    float stepSize = 0.0f;                      ///< Step interval for discrete parameters (0 = continuous).
    bool isDiscrete = false;                    ///< True if the parameter has a finite set of steps.
    ParameterControlType type = ParameterControlType::Continuous; ///< Control widget type (slider, combo, toggle).
    int stepCount = 0;                          ///< Number of discrete steps (0 if continuous).
    std::vector<std::string> stepNames;         ///< Display names for each discrete step.

    // Equality operator
    bool operator==(const PluginParameterInfo& other) const
    {
        return index == other.index &&
            id == other.id &&
            name == other.name &&
            defaultValue == other.defaultValue &&
            currentValue == other.currentValue &&
            label == other.label &&
            isAutomatable == other.isAutomatable &&
            isRemoteControllable == other.isRemoteControllable &&
            category == other.category &&
            minValue == other.minValue &&
            maxValue == other.maxValue &&
            stepSize == other.stepSize &&
            isDiscrete == other.isDiscrete &&
            type == other.type &&
            stepCount == other.stepCount;
    }

    // Inequality operator
    bool operator!=(const PluginParameterInfo& other) const
    {
        return !(*this == other);
    }

    // Less-than operator (primarily based on index, then id)
    bool operator<(const PluginParameterInfo& other) const
    {
        if (index != other.index)
            return index < other.index;
        return id < other.id;
    }

    // Greater-than operator
    bool operator>(const PluginParameterInfo& other) const
    {
        return other < *this;
    }

    // Less-than-or-equal operator
    bool operator<=(const PluginParameterInfo& other) const
    {
        return !(other < *this);
    }

    // Greater-than-or-equal operator
    bool operator>=(const PluginParameterInfo& other) const
    {
        return !(*this < other);
    }

    juce::String toString() const
    {
        return juce::String(index) + ";"
            + id + ";"
            + name + ";"
            + juce::String(defaultValue) + ";"
            + juce::String(currentValue) + ";"
            + label + ";"
            + juce::String(isAutomatable ? 1 : 0) + ";"
            + juce::String(isRemoteControllable ? 1 : 0) + ";"
            + juce::String(static_cast<int>(category)) + ";"
            + juce::String(minValue) + ";"
            + juce::String(maxValue) + ";"
            + juce::String(stepSize) + ";"
            + juce::String(isDiscrete ? 1 : 0) + ";"
            + juce::String(int(type)) + ";"
            + juce::String(stepCount) + ";"
            + [&]() {
                juce::StringArray sa;
                for (const auto& s : stepNames)
                    sa.add(s);
                return sa.joinIntoString(",");
            }();
    }
    bool initializeFromString(const juce::String& parameterString)
    {
        juce::StringArray parameterStringArray;
        auto paramCnt = parameterStringArray.addTokens(parameterString, ";", "");
        if (16 == paramCnt)
        {
            index = parameterStringArray[0].getIntValue();
            id = parameterStringArray[1];
            name = parameterStringArray[2];
            defaultValue = parameterStringArray[3].getFloatValue();
            currentValue = parameterStringArray[4].getFloatValue();
            label = parameterStringArray[5];
            isAutomatable = parameterStringArray[6].getIntValue() == 1 ? true : false;
            isRemoteControllable = parameterStringArray[7].getIntValue() == 1 ? true : false;
            category = static_cast<juce::AudioProcessorParameter::Category>(parameterStringArray[8].getIntValue());
            minValue = parameterStringArray[9].getFloatValue();
            maxValue = parameterStringArray[10].getFloatValue();
            stepSize = parameterStringArray[11].getFloatValue();
            isDiscrete = parameterStringArray[12].getIntValue() == 1 ? true : false;
            type = static_cast<ParameterControlType>(parameterStringArray[13].getIntValue());
            stepCount = parameterStringArray[14].getIntValue();

            juce::StringArray stepNamesArray;
            stepNamesArray.addTokens(parameterStringArray[15], ",", "");
            for (auto const& stepName : stepNamesArray)
                stepNames.push_back(stepName.toStdString());
            jassert(stepCount == stepNames.size());

            return true;
        }

        return false;
    }
    static PluginParameterInfo fromString(const juce::String& parameterString)
    {
        PluginParameterInfo parameterInfo;
        auto success = parameterInfo.initializeFromString(parameterString);
        jassert(success);
        return parameterInfo;
    };
    bool initializeFromAudioProcessorParameter(juce::AudioProcessorParameter& processorParameter)
    {
        index = processorParameter.getParameterIndex();
        name = processorParameter.getName(100);
        label = processorParameter.getLabel();
        defaultValue = processorParameter.getDefaultValue();
        currentValue = processorParameter.getValue();
        isAutomatable = processorParameter.isAutomatable();
        isRemoteControllable = false;
        category = processorParameter.getCategory();

        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(&processorParameter))
        {
            id = paramWithID->paramID;
        }
        else
        {
            id = "param_" + juce::String(index);
        }

        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*>(&processorParameter))
        {
            auto range = rangedParam->getNormalisableRange();
            minValue = range.start;
            maxValue = range.end;
            stepSize = range.interval;
            isDiscrete = range.interval > 0.0f;
        }
        else
        {
            minValue = 0.0f;
            maxValue = 1.0f;
            stepSize = 0.0f;
            isDiscrete = false;
        }

        return true;
    }
    static PluginParameterInfo fromAudioProcessorParameter(juce::AudioProcessorParameter& processorParameter)
    {
        PluginParameterInfo parameterInfo;
        auto success = parameterInfo.initializeFromAudioProcessorParameter(processorParameter);
        jassert(success);
        return parameterInfo;
    };

    static std::vector<PluginParameterInfo> parametersToInfos(juce::Array<juce::AudioProcessorParameter*> processorParameters)
    {
        std::vector<PluginParameterInfo> infos(processorParameters.size());
        for (auto param : processorParameters)
            infos.push_back(fromAudioProcessorParameter(*param));
        return infos;
    };
};

} // namespace Mema
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

struct PluginParameterInfo
{
    int index = 0;
    juce::String id = "0";
    juce::String name = "";
    float defaultValue = 0.0f;
    float currentValue = 0.0f;
    juce::String label = "";
    bool isAutomatable = false;
    bool isRemoteControllable = true;  // User-controllable remote access
    juce::AudioProcessorParameter::Category category = juce::AudioProcessorParameter::Category::genericParameter;
    // Range information
    float minValue = 0.0f;
    float maxValue = 1.0f;
    float stepSize = 0.0f;
    bool isDiscrete = false;

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
            isDiscrete == other.isDiscrete;
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
            + juce::String(isDiscrete ? 1 : 0);
    }
    bool initializeFromString(const juce::String& parameterString)
    {
        juce::StringArray parameterStringArray;
        auto paramCnt = parameterStringArray.addTokens(parameterString, ";", "");
        if (13 == paramCnt)
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
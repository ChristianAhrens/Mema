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


//==============================================================================
/*
*/
class ADMOSController : public juce::OSCReceiver::Listener<juce::OSCReceiver::MessageLoopCallback>
{
public:
    enum ADMOSCParameterChangeTarget
    {
        None, // change should only be cached and not forwarded in any way
        Internal, // change is to be sent from this app/process to outside world
        External,  // change was received from outside world and is to be distributed internally
    };
    enum ADMOSCParameterType
    {
        Empty = 0,
        X,
        Y,
        Z,
        XY,
        XYZ,
        Width,
        Mute
    };
    struct ADMOSCParameter
    {
        ADMOSCParameter() = default;
        ADMOSCParameter(ADMOSCParameterType t) {
            type = std::uint16_t(t);
            switch (type)
            {
            case ADMOSCParameterType::X:
            case ADMOSCParameterType::Y:
            case ADMOSCParameterType::Z:
            case ADMOSCParameterType::Width:
            case ADMOSCParameterType::Mute:
                parameterCount = 1;
                break;
            case ADMOSCParameterType::XY:
                parameterCount = 2;
                break;
            case ADMOSCParameterType::XYZ:
                parameterCount = 3;
                break;
            case ADMOSCParameterType::Empty:
            default:
                parameterCount = 0;
                jassertfalse;
                break;
            }
        };

        std::uint16_t   type = ADMOSCParameterType::Empty;
        std::uint16_t   parameterCount = 0;
        std::uint32_t   parameter1 = 0;
        std::uint32_t   parameter2 = 0;
        std::uint32_t   parameter3 = 0;
    };
    struct ADMOSCParameterX : public ADMOSCParameter
    {
        ADMOSCParameterX() {
            type = ADMOSCParameterType::X;
            parameterCount = 1;
        };
        ADMOSCParameterX(const ADMOSCParameter& other) : ADMOSCParameterX() {
            jassert(ADMOSCParameterType::X == other.type || ADMOSCParameterType::XY == other.type || ADMOSCParameterType::XYZ == other.type);
            parameter1 = other.parameter1;
            parameter2 = 0;
            parameter3 = 0;
        };
        ADMOSCParameterX(float xVal) : ADMOSCParameterX() {
            std::memcpy(&parameter1, &xVal, sizeof(parameter1));
        };
        float getParameterVal() {
            float val;
            std::memcpy(&val, &parameter1, sizeof(val));
            return val;
        };
    };
    struct ADMOSCParameterY : public ADMOSCParameter
    {
        ADMOSCParameterY() {
            type = ADMOSCParameterType::Y;
            parameterCount = 1;
        };
        ADMOSCParameterY(const ADMOSCParameter& other) : ADMOSCParameterY() {
            jassert(ADMOSCParameterType::Y == other.type || ADMOSCParameterType::XY == other.type || ADMOSCParameterType::XYZ == other.type);
            parameter1 = 0;
            parameter2 = other.parameter2;
            parameter3 = 0;
        };
        ADMOSCParameterY(float yVal) : ADMOSCParameterY() {
            std::memcpy(&parameter2, &yVal, sizeof(parameter2));
        };
        float getParameterVal() {
            float val;
            std::memcpy(&val, &parameter2, sizeof(val));
            return val;
        };
    };
    struct ADMOSCParameterZ : public ADMOSCParameter
    {
        ADMOSCParameterZ() {
            type = ADMOSCParameterType::Z;
            parameterCount = 1;
        };
        ADMOSCParameterZ(const ADMOSCParameter& other) : ADMOSCParameterZ() {
            jassert(ADMOSCParameterType::Z == other.type || ADMOSCParameterType::XYZ == other.type);
            parameter1 = 0;
            parameter2 = 0;
            parameter3 = other.parameter3;
        };
        ADMOSCParameterZ(float zVal) : ADMOSCParameterZ() {
            std::memcpy(&parameter3, &zVal, sizeof(parameter3));
        };
        float getParameterVal() {
            float val;
            std::memcpy(&val, &parameter3, sizeof(val));
            return val;
        };
    };
    struct ADMOSCParameterXY : public ADMOSCParameter
    {
        ADMOSCParameterXY() {
            type = ADMOSCParameterType::XY;
            parameterCount = 2;
        };
        ADMOSCParameterXY(const ADMOSCParameter& other) : ADMOSCParameterXY() {
            jassert(ADMOSCParameterType::XY == other.type || ADMOSCParameterType::XYZ == other.type);
            parameter1 = other.parameter1;
            parameter2 = other.parameter2;
            parameter3 = 0;
        };
        ADMOSCParameterXY(float xVal, float yVal) : ADMOSCParameterXY() {
            std::memcpy(&parameter1, &xVal, sizeof(parameter1));
            std::memcpy(&parameter2, &yVal, sizeof(parameter2));
        };
        std::tuple<float, float> getParameterVals() {
            std::tuple<float, float> val;
            std::memcpy(&std::get<0>(val), &parameter1, sizeof(std::get<0>(val)));
            std::memcpy(&std::get<1>(val), &parameter2, sizeof(std::get<1>(val)));
            return val;
        };
    };
    struct ADMOSCParameterXYZ : public ADMOSCParameter
    {
        ADMOSCParameterXYZ() {
            type = ADMOSCParameterType::XYZ;
            parameterCount = 3;
        };
        ADMOSCParameterXYZ(const ADMOSCParameter& other) : ADMOSCParameterXYZ() {
            jassert(ADMOSCParameterType::XYZ == other.type);
            parameter1 = other.parameter1;
            parameter2 = other.parameter2;
            parameter3 = other.parameter3;
        };
        ADMOSCParameterXYZ(float xVal, float yVal, float zVal) : ADMOSCParameterXYZ() {
            std::memcpy(&parameter1, &xVal, sizeof(parameter1));
            std::memcpy(&parameter2, &yVal, sizeof(parameter2));
            std::memcpy(&parameter3, &zVal, sizeof(parameter3));
        };
        std::tuple<float, float, float> getParameterVals() {
            std::tuple<float, float, float> val;
            std::memcpy(&std::get<0>(val), &parameter1, sizeof(std::get<0>(val)));
            std::memcpy(&std::get<1>(val), &parameter2, sizeof(std::get<1>(val)));
            std::memcpy(&std::get<2>(val), &parameter3, sizeof(std::get<2>(val)));
            return val;
        };
    };
    struct ADMOSCParameterWidth : public ADMOSCParameter
    {
        ADMOSCParameterWidth() {
            type = ADMOSCParameterType::Width;
            parameterCount = 1;
        };
        ADMOSCParameterWidth(const ADMOSCParameter& other) : ADMOSCParameterWidth() {
            jassert(ADMOSCParameterType::Width == other.type);
            parameter1 = other.parameter1;
            parameter2 = 0;
            parameter3 = 0;
        };
        ADMOSCParameterWidth(float width) : ADMOSCParameterWidth() {
            std::memcpy(&parameter1, &width, sizeof(parameter1));
        };
        float getParameterVal() {
            float val;
            std::memcpy(&val, &parameter1, sizeof(val));
            return val;
        };
    };
    struct ADMOSCParameterMute : public ADMOSCParameter
    {
        ADMOSCParameterMute() {
            type = ADMOSCParameterType::Mute;
            parameterCount = 1;
        };
        ADMOSCParameterMute(const ADMOSCParameter& other) : ADMOSCParameterMute() {
            jassert(ADMOSCParameterType::Mute == other.type);
            parameter1 = other.parameter1;
            parameter2 = 0;
            parameter3 = 0;
        };
        ADMOSCParameterMute(bool mute) : ADMOSCParameterMute() {
            parameter1 = mute ? 1 : 0;
        };
        ADMOSCParameterMute(int mute01) : ADMOSCParameterMute() {
            parameter1 = mute01;
        };
        bool getParameterVal() { return 1 == parameter1 ? true : false; };
        int getParameterVal01() { return int(parameter1); };
    };

public:
    ADMOSController();
    ~ADMOSController();

    bool startConnection(int oscPort, juce::IPAddress targetIP, int targetPort);

    void setNumObjects(int numObjects);

    void setParameter(int objNum, const ADMOSCParameter& param, const ADMOSCParameterChangeTarget& pct = ADMOSCParameterChangeTarget::None);
    ADMOSCParameter getParameter(int objNum, std::uint16_t type);
    
    //==============================================================================
    void oscMessageReceived (const juce::OSCMessage& message) override;
    void oscBundleReceived(const juce::OSCBundle& bundle) override;

    //==============================================================================
    std::function<void(int, std::uint16_t)> onParameterChanged;
    
protected:
    //==============================================================================
    const std::vector<int>  getObjNumsFromObjIdent(const juce::String& objIdent);
    
    bool sendParameterChange(int objNum, const ADMOSController::ADMOSCParameter& param);

    const juce::OSCMessage getParameterAsOSCMessage(int objNum, const ADMOSController::ADMOSCParameter& param);
    
private:
    //==============================================================================
    const juce::String s_admObjDomainStr = "/adm/obj/";
    const juce::String s_xStr= "/x";
    const juce::String s_yStr= "/y";
    const juce::String s_zStr= "/z";
    const juce::String s_xyStr= "/xy";
    const juce::String s_xyzStr= "/xyz";
    const juce::String s_widthStr = "/w";
    const juce::String s_muteStr= "/mute";
    
    //==============================================================================
    std::unique_ptr<juce::OSCReceiver>  m_oscReceiver;
    std::unique_ptr<juce::OSCSender>    m_oscSender;

    //==============================================================================
    std::vector<int>    m_knownObjNums;
    std::map<int, std::map<std::uint16_t, ADMOSCParameter>>  m_objCache;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ADMOSController)
};

}
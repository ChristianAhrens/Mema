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

#include "ADMOSController.h"


namespace Mema
{


std::mutex  ADMOSController::ADMOSCParameterChangedMessage::m_typeMapMutex;
std::map<int, std::vector<std::uint16_t>>   ADMOSController::ADMOSCParameterChangedMessage::m_typeMap;


//==============================================================================
ADMOSController::ADMOSController()
{
    m_oscReceiver = std::make_unique<juce::OSCReceiver>("ADM-OSC communication thread");
    m_oscReceiver->registerFormatErrorHandler([=](const char* /*data*/, int dataSize) { DBG(juce::String(__FUNCTION__) + " received " + juce::String(dataSize) + " bytes of unknown data"); });
    m_oscReceiver->addListener(this);

    m_oscSender = std::make_unique<juce::OSCSender>();
}

ADMOSController::~ADMOSController()
{
}

bool ADMOSController::startConnection(int receiverOscPort, juce::IPAddress targetIP, int targetPort)
{
    DBG(juce::String(__FUNCTION__) << " RP:" << receiverOscPort << " TIP:" << targetIP.toString() << " TP:" << targetPort);

    auto success = true;
    if (m_oscReceiver)
    {
        auto connected = m_oscReceiver->connect(receiverOscPort);
        if (!connected)
            juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Communication issue", "ADM-OSC port " + juce::String(receiverOscPort) + " could not be opened.", "Ok");
        success = success && connected;
    }
    else
        success = false;

    if (m_oscSender)
    {
        auto connected = m_oscSender->connect(targetIP.toString(), targetPort);
        if (!connected)
            juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon, "Communication issue", "ADM-OSC connection to given " + targetIP.toString() + ":" + juce::String(targetPort) + " could not be established.", "Ok");
        success = success && connected;
    }
    else
        success = false;

    return success;
}

void ADMOSController::setNumObjects(int numObjects)
{
    m_knownObjNums.resize(numObjects);
    for (int i = 1; i <= numObjects; i++)
    {
        m_knownObjNums[i-1] = i;
        for (std::uint16_t t = ADMOSCParameterType::X; t <= ADMOSCParameterType::Mute; t++)
            m_objCache[i][t] = ADMOSCParameter(ADMOSCParameterType(t));
    }
}

const std::vector<int> ADMOSController::getObjNumsFromObjIdent(const juce::String& objIdent)
{
    auto objNums = std::vector<int>();
    if (!objIdent.containsAnyOf("*?{}[]"))
        objNums.push_back(objIdent.getIntValue());
    else
    {
        if (objIdent == "*")
        {
            objNums = m_knownObjNums;
        }
        else if (objIdent.startsWith("{") && objIdent.endsWith("}"))
        {
            auto numListStr = objIdent.substring(1, objIdent.length());
            juce::StringArray nsa;
            nsa.addTokens(numListStr, ",", "");
            objNums.reserve(nsa.size());
            for (auto const& ns : nsa)
                objNums.push_back(ns.getIntValue());
        }
        else if (objIdent.startsWith("[") && objIdent.endsWith("]"))
        {
            auto numListStr = objIdent.substring(1, objIdent.length());
            juce::StringArray nsa;
            nsa.addTokens(numListStr, "-", "");
            jassert(2 == nsa.size());
            auto startNum = nsa[0].getIntValue();
            auto endNum = nsa[1].getIntValue();
            objNums.reserve(endNum - startNum + 1);
            for (auto i = startNum; i <= endNum; i++)
                objNums.push_back(i);
        }
        else
        {
            DBG(juce::String(__FUNCTION__) + " OSC address ident " + objIdent + " not supported.");
        }
    }

    return objNums;
}

void ADMOSController::setParameter(int objNum, const ADMOSController::ADMOSCParameter& param, const ADMOSCParameterChangeTarget& pct)
{
//#define TESTING
#ifdef TESTING
    switch (param.type)
    {
    case ADMOSController::ADMOSCParameterType::X:
        DBG(juce::String(__FUNCTION__) << " type X: " << ADMOSController::ADMOSCParameterX(param).getParameterVal());
        break;
    case ADMOSController::ADMOSCParameterType::Y:
        DBG(juce::String(__FUNCTION__) << " type Y: " << ADMOSController::ADMOSCParameterY(param).getParameterVal());
        break;
    case ADMOSController::ADMOSCParameterType::Z:
        DBG(juce::String(__FUNCTION__) << " type Z: " << ADMOSController::ADMOSCParameterZ(param).getParameterVal());
        break;
    case ADMOSController::ADMOSCParameterType::XY:
        {
            auto xyVals = ADMOSController::ADMOSCParameterXY(param).getParameterVals();
            DBG(juce::String(__FUNCTION__) << " type XY: " << std::get<0>(xyVals) << " " << std::get<1>(xyVals));
        }
        break;
    case ADMOSController::ADMOSCParameterType::XYZ:
        {
            auto xyzVals = ADMOSController::ADMOSCParameterXYZ(param).getParameterVals();
            DBG(juce::String(__FUNCTION__) << " type XYZ: " << std::get<0>(xyzVals) << " " << std::get<1>(xyzVals) << " " << std::get<2>(xyzVals));
        }
        break;
    case ADMOSController::ADMOSCParameterType::Width:
        DBG(juce::String(__FUNCTION__) << " type Width: " << ADMOSController::ADMOSCParameterWidth(param).getParameterVal());
        break;
    case ADMOSController::ADMOSCParameterType::Mute:
        DBG(juce::String(__FUNCTION__) << " type Mute: " << ADMOSController::ADMOSCParameterMute(param).getParameterVal01());
        break;
    case ADMOSController::ADMOSCParameterType::Empty:
    default:
        DBG(juce::String(__FUNCTION__) + " type NONE unsupported.");
        jassertfalse;
        break;
    }
#endif

    if (ADMOSController::ADMOSCParameterType::Empty == param.type)
    {
        jassertfalse;
        return;
    }
    else
    {
        {
            std::lock_guard<std::mutex> l(m_objCacheMutex);
            m_objCache[objNum][param.type] = param;
        }
        ADMOSCParameterChangedMessage::createAndPostIfNotAlreadyPending(objNum, param.type, pct, this);
    }
}

void ADMOSController::handleMessage(const Message& message)
{
    if (const ADMOSCParameterChangedMessage* apcm = dynamic_cast<const ADMOSCParameterChangedMessage*>(&message))
    {
        if (ADMOSCParameterChangeTarget::Internal == apcm->getTarget() && onParameterChanged)
            onParameterChanged(apcm->getObjNum(), apcm->getType());
        else if (ADMOSCParameterChangeTarget::External == apcm->getTarget())
            sendParameterChange(apcm->getObjNum(), getParameter(apcm->getObjNum(), apcm->getType()));
    }
}

ADMOSController::ADMOSCParameter ADMOSController::getParameter(int objNum, std::uint16_t type)
{
    if (ADMOSController::ADMOSCParameterType::Empty == type || 0 == m_objCache.count(objNum) || 0 == m_objCache[objNum].count(type))
    {
        jassertfalse;
        return {};
    }
    else
    {
        auto parameter = ADMOSCParameter();
        {
            std::lock_guard<std::mutex> l(m_objCacheMutex);
            parameter = m_objCache[objNum][type];
        }
        return parameter;
    }
}

bool ADMOSController::sendParameterChange(int objNum, const ADMOSController::ADMOSCParameter& param)
{
    if (m_oscSender)
    {
#ifdef DEBUG
        auto msg = getParameterAsOSCMessage(objNum, param);
        juce::String msgAsStr = msg.getAddressPattern().toString() + " ";
        for (auto const& arg : msg)
            switch (arg.getType())
            {
            case 'i':
                msgAsStr += juce::String(arg.getInt32()) + " ";
                break;
            case 'f':
                msgAsStr += juce::String(arg.getFloat32()) + " ";
                break;
            default:
                break;
            }
        DBG(juce::String(__FUNCTION__) + " " + msgAsStr);
        return m_oscSender->send(msg);
#else
        return m_oscSender->send(getParameterAsOSCMessage(objNum, param));
#endif
    }
    else
        return false;
}

const juce::OSCMessage ADMOSController::getParameterAsOSCMessage(int objNum, const ADMOSController::ADMOSCParameter& param)
{
    switch (param.type)
    {
    case ADMOSController::ADMOSCParameterType::X:
        return juce::OSCMessage(juce::OSCAddressPattern(s_admObjDomainStr + juce::String(objNum) + s_xStr), ADMOSCParameterX(param).getParameterVal());
    case ADMOSController::ADMOSCParameterType::Y:
        return juce::OSCMessage(juce::OSCAddressPattern(s_admObjDomainStr + juce::String(objNum) + s_yStr), ADMOSCParameterY(param).getParameterVal());
    case ADMOSController::ADMOSCParameterType::Z:
        return juce::OSCMessage(juce::OSCAddressPattern(s_admObjDomainStr + juce::String(objNum) + s_zStr), ADMOSCParameterZ(param).getParameterVal());
    case ADMOSController::ADMOSCParameterType::XY:
    {
        auto xyVal = ADMOSCParameterXY(param).getParameterVals();
        return juce::OSCMessage(juce::OSCAddressPattern(s_admObjDomainStr + juce::String(objNum) + s_xyStr), std::get<0>(xyVal), std::get<1>(xyVal));
    }
    case ADMOSController::ADMOSCParameterType::XYZ:
    {
        auto xyzVal = ADMOSCParameterXYZ(param).getParameterVals();
        return juce::OSCMessage(juce::OSCAddressPattern(s_admObjDomainStr + juce::String(objNum) + s_xyzStr), std::get<0>(xyzVal), std::get<1>(xyzVal), std::get<2>(xyzVal));
    }
    case ADMOSController::ADMOSCParameterType::Width:
        return juce::OSCMessage(juce::OSCAddressPattern(s_admObjDomainStr + juce::String(objNum) + s_widthStr), ADMOSCParameterWidth(param).getParameterVal());
    case ADMOSController::ADMOSCParameterType::Mute:
        return juce::OSCMessage(juce::OSCAddressPattern(s_admObjDomainStr + juce::String(objNum) + s_muteStr), ADMOSCParameterMute(param).getParameterVal01());
    case ADMOSController::ADMOSCParameterType::Empty:
    default:
        jassertfalse;
        return juce::OSCMessage(juce::OSCAddressPattern(s_admObjDomainStr + juce::String(objNum)));
    }
}

void ADMOSController::oscMessageReceived(const juce::OSCMessage& message)
{
    if (message.isEmpty())
        return;

    auto addrStr = message.getAddressPattern().toString();
    if (addrStr.startsWith(s_admObjDomainStr))
    {
        auto addrContents = addrStr.fromFirstOccurrenceOf(s_admObjDomainStr, false, false);

        auto objIdent = addrContents.upToFirstOccurrenceOf("/", false, false);
        auto objNums = getObjNumsFromObjIdent(objIdent);

        ADMOSCParameter param;
        if (addrContents.endsWith(s_xStr) && 1 == message.size())
            param = ADMOSCParameterX(message[0].getFloat32());
        else if (addrContents.endsWith(s_yStr) && 1 == message.size())
            param = ADMOSCParameterY(message[0].getFloat32());
        else if (addrContents.endsWith(s_zStr) && 1 == message.size())
            param = ADMOSCParameterZ(message[0].getFloat32());
        else if (addrContents.endsWith(s_xyStr) && 2 == message.size())
            param = ADMOSCParameterXY(message[0].getFloat32(), message[1].getFloat32());
        else if (addrContents.endsWith(s_xyzStr) && 3 == message.size())
            param = ADMOSCParameterXYZ(message[0].getFloat32(), message[1].getFloat32(), message[2].getFloat32());
        else if (addrContents.endsWith(s_widthStr) && 1 == message.size())
            param = ADMOSCParameterWidth(message[0].getFloat32());
        else if (addrContents.endsWith(s_muteStr) && 1 == message.size())
            param = ADMOSCParameterMute(message[0].getInt32());

        for (auto const& objNum : objNums)
            setParameter(objNum, param, ADMOSCParameterChangeTarget::Internal);
    }
    else
    {
        DBG(juce::String(__FUNCTION__) + " unhandled OSC message: " + addrStr + "(" + juce::String(message.size()) + " args)");
    }
}

void ADMOSController::oscBundleReceived(const juce::OSCBundle& bundle)
{
    if (bundle.isEmpty())
        return;
    else
    {
        for (auto const& element : bundle)
        {
            if (element.isBundle())
                oscBundleReceived(element.getBundle());
            else if (element.isMessage())
                oscMessageReceived(element.getMessage());
        }
    }
}


}

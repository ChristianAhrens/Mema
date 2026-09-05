/* Copyright (c) 2026, Christian Ahrens
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

// Coverage for Mema::ADMOSController -- the ADM-OSC protocol implementation behind
// Mema.Re's external panning control (see README's "Mema.Re ADM-OSC external
// control"). Deliberately never calls startConnection(): that's the only method
// that actually binds a UDP socket / starts a background thread (verified by
// reading JUCE's OSCReceiver::Pimpl -- its constructor is inert, startThread()
// only runs from connect()), so every test here only constructs the controller
// and exercises pure parsing/building/caching logic.
//
// setParameter() (called internally by oscMessageReceived()) does post an
// ADMOSCParameterChangedMessage via juce::MessageListener::postMessage(). The
// cache write it triggers happens synchronously before that post, so checking
// getParameter() right after calling oscMessageReceived() is a reliable way to
// verify address-pattern parsing without depending on a running message loop to
// ever actually deliver that notification.

#include <JuceHeader.h>
#include <MemaClientCommon/ADMOSController.h>

using namespace Mema;

//==============================================================================
class ADMOSCParameterBitPackingTest : public juce::UnitTest
{
public:
    ADMOSCParameterBitPackingTest() : juce::UnitTest ("ADMOSCParameter bit-packing round-trip", "Mema.Re") {}

    void runTest() override
    {
        beginTest ("X/Y/Z/Width round-trip a float value exactly");
        {
            expectEquals (ADMOSController::ADMOSCParameterX (0.25f).getParameterVal(), 0.25f);
            expectEquals (ADMOSController::ADMOSCParameterY (-0.5f).getParameterVal(), -0.5f);
            expectEquals (ADMOSController::ADMOSCParameterZ (1.0f).getParameterVal(), 1.0f);
            expectEquals (ADMOSController::ADMOSCParameterWidth (0.75f).getParameterVal(), 0.75f);
        }

        beginTest ("XY/XYZ round-trip both/all components exactly");
        {
            auto xy = ADMOSController::ADMOSCParameterXY (0.1f, 0.2f).getParameterVals();
            expectEquals (std::get<0> (xy), 0.1f);
            expectEquals (std::get<1> (xy), 0.2f);

            auto xyz = ADMOSController::ADMOSCParameterXYZ (0.1f, 0.2f, 0.3f).getParameterVals();
            expectEquals (std::get<0> (xyz), 0.1f);
            expectEquals (std::get<1> (xyz), 0.2f);
            expectEquals (std::get<2> (xyz), 0.3f);
        }

        beginTest ("Mute round-trips both bool and 0/1 int forms");
        {
            expect (ADMOSController::ADMOSCParameterMute (true).getParameterVal());
            expect (! ADMOSController::ADMOSCParameterMute (false).getParameterVal());
            expectEquals (ADMOSController::ADMOSCParameterMute (1).getParameterVal01(), 1);
            expectEquals (ADMOSController::ADMOSCParameterMute (0).getParameterVal01(), 0);
        }

        beginTest ("Narrowing conversions extract the matching component from a wider parameter");
        {
            // e.g. an XYZ update should still yield the correct Z value when narrowed to
            // just ADMOSCParameterZ -- this is how setParameter()'s cache-then-narrow usage works.
            ADMOSController::ADMOSCParameterXYZ xyz (0.4f, 0.5f, 0.6f);
            ADMOSController::ADMOSCParameterZ z (xyz);
            expectEquals (z.getParameterVal(), 0.6f);
        }
    }
};

static ADMOSCParameterBitPackingTest admOSCParameterBitPackingTest;

//==============================================================================
namespace
{
    // ADMOSController's constructor only creates OSCReceiver/OSCSender objects (no socket
    // bind, no thread start -- see file header note), so it's safe to construct directly.
    // getObjNumsFromObjIdent() and getParameterAsOSCMessage() are protected; expose them
    // for direct testing the same way MemaCommandersTest.cpp exposes protected triggers.
    class TestableADMOSController : public ADMOSController
    {
    public:
        using ADMOSController::getObjNumsFromObjIdent;
        using ADMOSController::getParameterAsOSCMessage;
    };
}

class ADMOSControllerObjIdentTest : public juce::UnitTest
{
public:
    ADMOSControllerObjIdentTest() : juce::UnitTest ("ADMOSController object identifier parsing", "Mema.Re") {}

    void runTest() override
    {
        TestableADMOSController controller;

        beginTest ("A plain numeric identifier resolves to a single object number");
        {
            auto objNums = controller.getObjNumsFromObjIdent ("3");
            expectEquals ((int) objNums.size(), 1);
            expectEquals (objNums[0], 3);
        }

        beginTest ("'*' resolves to every object registered via setNumObjects()");
        {
            controller.setNumObjects (4);
            auto objNums = controller.getObjNumsFromObjIdent ("*");
            expectEquals ((int) objNums.size(), 4);
            expect (std::find (objNums.begin(), objNums.end(), 1) != objNums.end());
            expect (std::find (objNums.begin(), objNums.end(), 4) != objNums.end());
        }

        beginTest ("A '{...}' list resolves to exactly the listed object numbers");
        {
            auto objNums = controller.getObjNumsFromObjIdent ("{1,3,5}");
            expectEquals ((int) objNums.size(), 3);
            expectEquals (objNums[0], 1);
            expectEquals (objNums[1], 3);
            expectEquals (objNums[2], 5);
        }

        beginTest ("A '[start-end]' range resolves to every number in that inclusive range");
        {
            auto objNums = controller.getObjNumsFromObjIdent ("[2-5]");
            expectEquals ((int) objNums.size(), 4);
            expectEquals (objNums[0], 2);
            expectEquals (objNums[3], 5);
        }
    }
};

static ADMOSControllerObjIdentTest admOSCControllerObjIdentTest;

//==============================================================================
class ADMOSControllerOutgoingMessageTest : public juce::UnitTest
{
public:
    ADMOSControllerOutgoingMessageTest() : juce::UnitTest ("ADMOSController outgoing OSC message building", "Mema.Re") {}

    void runTest() override
    {
        TestableADMOSController controller;

        beginTest ("XY parameter builds the /adm/obj/<n>/xy address with both arguments");
        {
            auto msg = controller.getParameterAsOSCMessage (2, ADMOSController::ADMOSCParameterXY (0.3f, 0.7f));
            expectEquals (msg.getAddressPattern().toString(), juce::String ("/adm/obj/2/xy"));
            expectEquals ((int) msg.size(), 2);
            expectEquals (msg[0].getFloat32(), 0.3f);
            expectEquals (msg[1].getFloat32(), 0.7f);
        }

        beginTest ("Mute parameter builds the /adm/obj/<n>/mute address with an int argument");
        {
            auto msg = controller.getParameterAsOSCMessage (5, ADMOSController::ADMOSCParameterMute (true));
            expectEquals (msg.getAddressPattern().toString(), juce::String ("/adm/obj/5/mute"));
            expectEquals ((int) msg.size(), 1);
            expectEquals (msg[0].getInt32(), 1);
        }

        beginTest ("Width parameter builds the /adm/obj/<n>/w address with a float argument");
        {
            auto msg = controller.getParameterAsOSCMessage (1, ADMOSController::ADMOSCParameterWidth (0.5f));
            expectEquals (msg.getAddressPattern().toString(), juce::String ("/adm/obj/1/w"));
            expectEquals (msg[0].getFloat32(), 0.5f);
        }
    }
};

static ADMOSControllerOutgoingMessageTest admOSCControllerOutgoingMessageTest;

//==============================================================================
class ADMOSControllerIncomingMessageTest : public juce::UnitTest
{
public:
    ADMOSControllerIncomingMessageTest() : juce::UnitTest ("ADMOSController incoming OSC message parsing", "Mema.Re") {}

    void runTest() override
    {
        beginTest ("An /xy message updates the cached XY parameter for the addressed object");
        {
            ADMOSController controller;
            juce::OSCMessage msg (juce::OSCAddressPattern ("/adm/obj/3/xy"), 0.25f, 0.75f);
            controller.oscMessageReceived (msg);

            auto cached = ADMOSController::ADMOSCParameterXY (controller.getParameter (3, ADMOSController::ADMOSCParameterType::XY));
            auto vals = cached.getParameterVals();
            expectEquals (std::get<0> (vals), 0.25f);
            expectEquals (std::get<1> (vals), 0.75f);
        }

        beginTest ("A /mute message updates the cached mute parameter for the addressed object");
        {
            ADMOSController controller;
            juce::OSCMessage msg (juce::OSCAddressPattern ("/adm/obj/2/mute"), (juce::int32) 1);
            controller.oscMessageReceived (msg);

            auto cached = ADMOSController::ADMOSCParameterMute (controller.getParameter (2, ADMOSController::ADMOSCParameterType::Mute));
            expect (cached.getParameterVal());
        }

        beginTest ("A wildcard object identifier applies the update to every known object");
        {
            ADMOSController controller;
            controller.setNumObjects (3);

            juce::OSCMessage msg (juce::OSCAddressPattern ("/adm/obj/*/x"), 0.42f);
            controller.oscMessageReceived (msg);

            for (int obj = 1; obj <= 3; ++obj)
            {
                auto cached = ADMOSController::ADMOSCParameterX (controller.getParameter (obj, ADMOSController::ADMOSCParameterType::X));
                expectEquals (cached.getParameterVal(), 0.42f);
            }
        }

        beginTest ("An unrecognised address pattern is ignored, not crashed on");
        {
            ADMOSController controller;
            juce::OSCMessage msg (juce::OSCAddressPattern ("/completely/unrelated/address"), 1.0f);
            controller.oscMessageReceived (msg); // should simply do nothing
        }
    }
};

static ADMOSControllerIncomingMessageTest admOSCControllerIncomingMessageTest;

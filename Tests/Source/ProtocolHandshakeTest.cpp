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

#include <JuceHeader.h>
#include <MemaProcessor/ProtocolHandshake.h>

using namespace Mema;

//==============================================================================
class ProtocolHandshakeTest : public juce::UnitTest
{
public:
    ProtocolHandshakeTest() : juce::UnitTest ("ProtocolHandshake::evaluate()", "Mema") {}

    void runTest() override
    {
        beginTest ("No pairing token configured on the server: any client is accepted, token or not");
        {
            expect (ProtocolHandshake::Result::Accept == ProtocolHandshake::evaluate ("", ""));
            expect (ProtocolHandshake::Result::Accept == ProtocolHandshake::evaluate ("", "anything"));
        }

        beginTest ("A pairing token is configured: a matching client token is accepted");
        {
            expect (ProtocolHandshake::Result::Accept == ProtocolHandshake::evaluate ("secret", "secret"));
        }

        beginTest ("A pairing token is configured: a mismatched or missing client token is rejected");
        {
            expect (ProtocolHandshake::Result::RejectToken == ProtocolHandshake::evaluate ("secret", "wrong"));
            expect (ProtocolHandshake::Result::RejectToken == ProtocolHandshake::evaluate ("secret", ""));
        }
    }
};

static ProtocolHandshakeTest protocolHandshakeTest;

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

#pragma once

#include <JuceHeader.h>


namespace Mema
{


/**
 * @class ProtocolHandshake
 * @brief Pure decision function for whether a client's `HelloMessage` should be accepted.
 *
 * @details Deliberately dependency-free (no socket, no `SerializableMessage`) so it can be
 * unit-tested in isolation from any live connection.
 *
 * @note There is no protocol-version parameter here. The wire protocol version travels in the
 * header of *every* message (see `SerializableMessage::s_protocolVersion`) and is already
 * checked by `SerializableMessage::initFromMemoryBlock()` before a `HelloMessage` is even
 * constructed -- by the time a `HelloMessage` reaches this function, version compatibility is
 * already established. Giving this function its own redundant version check would test a
 * combination that can't actually occur in the real connection flow. The pairing token is the
 * one thing the generic header check doesn't already cover, so it's the only thing evaluated here.
 */
class ProtocolHandshake
{
public:
    enum class Result
    {
        Accept,       ///< Pairing is disabled, or the client's token matches the server's.
        RejectToken   ///< The server has a pairing token configured and the client's didn't match.
    };

    /**
     * @brief Decides whether to accept a client's Hello.
     * @param serverToken The pairing token currently configured on the server. An empty string
     *                     means pairing is disabled -- any client is accepted.
     * @param clientToken The token the client declared in its `HelloMessage`.
     * @return `Result::Accept` or `Result::RejectToken`.
     */
    static Result evaluate(const juce::String& serverToken, const juce::String& clientToken)
    {
        if (!serverToken.isEmpty() && serverToken != clientToken)
            return Result::RejectToken;

        return Result::Accept;
    };
};


} // namespace Mema

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

// Coverage for the callback plumbing in MemaChannelCommander/MemaInputCommander/
// MemaOutputCommander/MemaCrosspointCommander/MemaPluginCommander (MemaCommanders.h).
// Each commander interface exposes a protected trigger method (e.g. inputMuteChange())
// that, when called by a concrete UI/network commander, should invoke the registered
// callback with a "sender" identity so the receiver can tell who originated the change
// (used for echo suppression -- see MemaProcessor::setInputMuteState()/setOutputMuteState()).
//
// Minimal concrete stub subclasses are used to construct these otherwise-abstract
// interfaces and expose their protected trigger methods for direct invocation.

#include <JuceHeader.h>
#include <MemaProcessor/MemaCommanders.h>

using namespace Mema;

namespace
{
    class StubInputCommander : public MemaInputCommander
    {
    public:
        void setChannelCount (std::uint16_t) override {}
        void setInputMute (std::uint16_t, bool, int = -1) override {}

        using MemaInputCommander::inputMuteChange;
        using MemaInputCommander::inputLevelChange;
        using MemaInputCommander::inputMutePoll;
        using MemaInputCommander::inputLevelPoll;
    };

    class StubOutputCommander : public MemaOutputCommander
    {
    public:
        void setChannelCount (std::uint16_t) override {}
        void setOutputMute (std::uint16_t, bool, int = -1) override {}

        using MemaOutputCommander::outputMuteChange;
        using MemaOutputCommander::outputLevelChange;
        using MemaOutputCommander::outputMutePoll;
        using MemaOutputCommander::outputLevelPoll;
    };

    class StubCrosspointCommander : public MemaCrosspointCommander
    {
    public:
        void setCrosspointEnabledValue (std::uint16_t, std::uint16_t, bool, int = -1) override {}
        void setCrosspointFactorValue (std::uint16_t, std::uint16_t, float, int = -1) override {}
        void setIOCount (std::uint16_t, std::uint16_t) override {}

        using MemaCrosspointCommander::crosspointEnabledChange;
        using MemaCrosspointCommander::crosspointEnabledPoll;
        using MemaCrosspointCommander::crosspointFactorChange;
        using MemaCrosspointCommander::crosspointFactorPoll;
    };

    class StubPluginCommander : public MemaPluginCommander
    {
    public:
        void setPluginParameterInfos (const std::vector<PluginParameterInfo>&, const std::string&, bool, bool, int = -1) override {}
        void setPluginParameterValue (std::uint16_t, std::string, float, int = -1) override {}
        void setPluginProcessingState (bool, bool, int = -1) override {}

        using MemaPluginCommander::pluginParameterInfosChange;
        using MemaPluginCommander::pluginParameterInfosPoll;
        using MemaPluginCommander::pluginParameterValueChange;
        using MemaPluginCommander::pluginParameterValuePoll;
        using MemaPluginCommander::pluginProcessingStateChange;
    };
}

class MemaInputCommanderTest : public juce::UnitTest
{
public:
    MemaInputCommanderTest() : juce::UnitTest ("MemaInputCommander callback plumbing", "Mema") {}

    void runTest() override
    {
        beginTest ("inputMuteChange() reports itself as sender (ignores the sender argument it was called with)");

        StubInputCommander commander;
        MemaInputCommander* gotSender = nullptr;
        std::uint16_t gotChannel = 0;
        bool gotMute = false;

        commander.setInputMuteChangeCallback ([&] (MemaInputCommander* sender, std::uint16_t channel, bool muted)
        {
            gotSender = sender;
            gotChannel = channel;
            gotMute = muted;
        });

        // Deliberately pass nullptr as the "sender" parameter to prove the implementation
        // reports `this`, not whatever was passed in.
        commander.inputMuteChange (5, true, nullptr);

        expect (gotSender == &commander, "callback should receive the commander instance itself as sender");
        expectEquals ((int) gotChannel, 5);
        expect (gotMute);
    }
};

static MemaInputCommanderTest memaInputCommanderTest;

//==============================================================================
class MemaOutputCommanderTest : public juce::UnitTest
{
public:
    MemaOutputCommanderTest() : juce::UnitTest ("MemaOutputCommander callback plumbing [bug]", "Mema") {}

    void runTest() override
    {
        beginTest ("outputMuteChange() reports nullptr as sender regardless of the actual originator [bug]");

        // MemaOutputCommander::outputMuteChange()/outputLevelChange()/outputMutePoll()/
        // outputLevelPoll() all hardcode `nullptr` as the sender passed to the callback,
        // instead of forwarding `this` (like MemaInputCommander) or the `sender` parameter
        // (like MemaCrosspointCommander). This defeats the echo-suppression check in
        // MemaProcessor::setOutputMuteState() -- `outputCommander != reinterpret_cast<...>(sender)`
        // can never match a real commander, so the originating UI commander is always
        // re-notified of its own change. In OutputControlComponent::setOutputMute() this is
        // currently harmless (it uses juce::dontSendNotification, so no feedback loop), but the
        // sender identity is unusable for anything relying on it. Documenting current (buggy)
        // behaviour here; see MemaCommanders.cpp for the suggested fix (mirror
        // MemaInputCommander and pass `this`).
        StubOutputCommander commander;
        MemaOutputCommander* gotSender = &commander; // pre-seed with a non-null sentinel

        commander.setOutputMuteChangeCallback ([&] (MemaOutputCommander* sender, std::uint16_t, bool)
        {
            gotSender = sender;
        });

        commander.outputMuteChange (5, true, &commander);

        expect (gotSender == nullptr, "current (buggy) behaviour: sender always arrives as nullptr");
    }
};

static MemaOutputCommanderTest memaOutputCommanderTest;

//==============================================================================
class MemaCrosspointCommanderTest : public juce::UnitTest
{
public:
    MemaCrosspointCommanderTest() : juce::UnitTest ("MemaCrosspointCommander callback plumbing", "Mema") {}

    void runTest() override
    {
        beginTest ("crosspointEnabledChange() forwards the actual sender argument");

        StubCrosspointCommander commander;
        MemaCrosspointCommander* gotSender = nullptr;
        std::uint16_t gotIn = 0, gotOut = 0;
        bool gotState = false;

        commander.setCrosspointEnabledChangeCallback ([&] (MemaCrosspointCommander* sender, std::uint16_t in, std::uint16_t out, bool enabled)
        {
            gotSender = sender;
            gotIn = in;
            gotOut = out;
            gotState = enabled;
        });

        commander.crosspointEnabledChange (2, 3, true, &commander);

        expect (gotSender == &commander, "sender argument should be forwarded as-is");
        expectEquals ((int) gotIn, 2);
        expectEquals ((int) gotOut, 3);
        expect (gotState);

        beginTest ("crosspointFactorChange() forwards the actual sender argument");

        float gotFactor = 0.0f;
        commander.setCrosspointFactorChangeCallback ([&] (MemaCrosspointCommander* sender, std::uint16_t in, std::uint16_t out, float factor)
        {
            gotSender = sender;
            gotIn = in;
            gotOut = out;
            gotFactor = factor;
        });

        commander.crosspointFactorChange (4, 5, 0.25f, nullptr);

        expect (gotSender == nullptr, "sender argument (including nullptr) should be forwarded as-is");
        expectEquals ((int) gotIn, 4);
        expectEquals ((int) gotOut, 5);
        expectEquals (gotFactor, 0.25f);
    }
};

static MemaCrosspointCommanderTest memaCrosspointCommanderTest;

//==============================================================================
class MemaPluginCommanderTest : public juce::UnitTest
{
public:
    MemaPluginCommanderTest() : juce::UnitTest ("MemaPluginCommander callback plumbing", "Mema") {}

    void runTest() override
    {
        beginTest ("pluginParameterValueChange() forwards index, id, value and sender");

        StubPluginCommander commander;
        MemaPluginCommander* gotSender = nullptr;
        std::uint16_t gotIndex = 0;
        std::string gotId;
        float gotValue = 0.0f;

        commander.setPluginParameterValueChangeCallback ([&] (MemaPluginCommander* sender, std::uint16_t index, std::string id, float value)
        {
            gotSender = sender;
            gotIndex = index;
            gotId = id;
            gotValue = value;
        });

        commander.pluginParameterValueChange (3, "cutoff", 0.6f, &commander);

        expect (gotSender == &commander);
        expectEquals ((int) gotIndex, 3);
        expectEquals (juce::String (gotId), juce::String ("cutoff"));
        expectEquals (gotValue, 0.6f);

        beginTest ("pluginProcessingStateChange() forwards enabled/post and sender");

        bool gotEnabled = false, gotPost = false;
        commander.setPluginProcessingStateChangeCallback ([&] (MemaPluginCommander* sender, bool enabled, bool post)
        {
            gotSender = sender;
            gotEnabled = enabled;
            gotPost = post;
        });

        commander.pluginProcessingStateChange (true, false, &commander);

        expect (gotSender == &commander);
        expect (gotEnabled);
        expect (! gotPost);
    }
};

static MemaPluginCommanderTest memaPluginCommanderTest;

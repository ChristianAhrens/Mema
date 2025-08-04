/* Copyright (c) 2024, Christian Ahrens
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


class MemaChannelCommander
{
public:
    MemaChannelCommander();
    virtual ~MemaChannelCommander();

    virtual void setChannelCount(std::uint16_t channelCount) = 0;

protected:

private:
};

class MemaInputCommander : public MemaChannelCommander
{
public:
    MemaInputCommander();
    virtual ~MemaInputCommander() override;

    void setInputMuteChangeCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t, bool)>& callback);
    void setInputLevelChangeCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t, float)>& callback);
    void setInputMutePollCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t)>& callback);
    void setInputLevelPollCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t)>& callback);

    virtual void setInputMute(std::uint16_t channel, bool muteState, int userId = -1) = 0;
    virtual void setInputLevel(std::uint16_t channel, float levelValue, int userId = -1) { ignoreUnused(channel); ignoreUnused(levelValue); ignoreUnused(userId); };

protected:
    void inputMuteChange(std::uint16_t channel, bool muteState, MemaInputCommander* sender);
    void inputLevelChange(std::uint16_t channel, float levelValue, MemaInputCommander* sender);

    void inputMutePoll(std::uint16_t channel, MemaInputCommander* sender);
    void inputLevelPoll(std::uint16_t channel, MemaInputCommander* sender);

private:
    std::function<void(MemaInputCommander* sender, std::uint16_t, float)> m_inputLevelChangeCallback{ nullptr };
    std::function<void(MemaInputCommander* sender, std::uint16_t)>        m_inputLevelPollCallback{ nullptr };
    std::function<void(MemaInputCommander* sender, std::uint16_t, bool)>  m_inputMuteChangeCallback{ nullptr };
    std::function<void(MemaInputCommander* sender, std::uint16_t)>        m_inputMutePollCallback{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaInputCommander)
};

class MemaOutputCommander : public MemaChannelCommander
{
public:
    MemaOutputCommander();
    virtual ~MemaOutputCommander() override;

    void setOutputMuteChangeCallback(const std::function<void(MemaOutputCommander* sender, std::uint16_t, bool)>& callback);
    void setOutputLevelChangeCallback(const std::function<void(MemaOutputCommander* sender, std::uint16_t, float)>& callback);
    void setOutputMutePollCallback(const std::function<void(MemaOutputCommander* sender, std::uint16_t)>& callback);
    void setOutputLevelPollCallback(const std::function<void(MemaOutputCommander* sender, std::uint16_t)>& callback);

    virtual void setOutputMute(std::uint16_t channel, bool muteState, int userId = -1) = 0;
    virtual void setOutputLevel(std::uint16_t channel, float levelValue, int userId = -1) { ignoreUnused(channel); ignoreUnused(levelValue); ignoreUnused(userId);
    };

protected:
    void outputMuteChange(std::uint16_t channel, bool muteState, MemaOutputCommander* sender);
    void outputLevelChange(std::uint16_t channel, float levelValue, MemaOutputCommander* sender);

    void outputMutePoll(std::uint16_t channel, MemaOutputCommander* sender);
    void outputLevelPoll(std::uint16_t channel, MemaOutputCommander* sender);

private:
    std::function<void(MemaOutputCommander* sender, std::uint16_t, float)>    m_outputLevelChangeCallback{ nullptr };
    std::function<void(MemaOutputCommander* sender, std::uint16_t)>           m_outputLevelPollCallback{ nullptr };
    std::function<void(MemaOutputCommander* sender, std::uint16_t, bool)>     m_outputMuteChangeCallback{ nullptr };
    std::function<void(MemaOutputCommander* sender, std::uint16_t)>           m_outputMutePollCallback{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaOutputCommander)
};
    
class MemaCrosspointCommander : public MemaChannelCommander
{
public:
    MemaCrosspointCommander();
    virtual ~MemaCrosspointCommander() override;

    void setCrosspointEnabledChangeCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t, bool)>& callback);
    void setCrosspointEnabledPollCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t)>& callback);
    virtual void setCrosspointEnabledValue(std::uint16_t input, std::uint16_t output, bool enabledState, int userId = -1) = 0;

    void setCrosspointFactorChangeCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t, float)>& callback);
    void setCrosspointFactorPollCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t)>& callback);
    virtual void setCrosspointFactorValue(std::uint16_t input, std::uint16_t output, float factor, int userId = -1) = 0;

    virtual void setIOCount(std::uint16_t inputCount, std::uint16_t outputCount) = 0;

protected:
    void crosspointEnabledChange(std::uint16_t input, std::uint16_t output, bool enabledState, MemaCrosspointCommander* sender);
    void crosspointEnabledPoll(std::uint16_t input, std::uint16_t output, MemaCrosspointCommander* sender);

    void crosspointFactorChange(std::uint16_t input, std::uint16_t output, float factor, MemaCrosspointCommander* sender);
    void crosspointFactorPoll(std::uint16_t input, std::uint16_t output, MemaCrosspointCommander* sender);

private:
    void setChannelCount(std::uint16_t channelCount) override { ignoreUnused(channelCount); jassertfalse; };

    std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t, bool)> m_crosspointEnabledChangeCallback{ nullptr };
    std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t)>       m_crosspointEnabledPollCallback{ nullptr };

    std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t, float)>   m_crosspointFactorChangeCallback{ nullptr };
    std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t)>          m_crosspointFactorPollCallback{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaCrosspointCommander)
};


} // namespace Mema

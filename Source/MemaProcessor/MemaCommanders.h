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

    virtual void setChannelCount(int channelCount) = 0;

protected:

private:
};

class MemaInputCommander : public MemaChannelCommander
{
public:
    MemaInputCommander();
    virtual ~MemaInputCommander() override;

    void setInputMuteChangeCallback(const std::function<void(MemaInputCommander* sender, int, bool)>& callback);
    void setInputLevelChangeCallback(const std::function<void(MemaInputCommander* sender, int, float)>& callback);
    void setInputMutePollCallback(const std::function<void(MemaInputCommander* sender, int)>& callback);
    void setInputLevelPollCallback(const std::function<void(MemaInputCommander* sender, int)>& callback);

    virtual void setInputMute(unsigned int channel, bool muteState) = 0;
    virtual void setInputLevel(unsigned int channel, float levelValue) { ignoreUnused(channel); ignoreUnused(levelValue); };

protected:
    void inputMuteChange(int channel, bool muteState);
    void inputLevelChange(int channel, float levelValue);
        
    void inputMutePoll(int channel);
    void inputLevelPoll(int channel);

private:
    std::function<void(MemaInputCommander* sender, int, float)> m_inputLevelChangeCallback{ nullptr };
    std::function<void(MemaInputCommander* sender, int)>        m_inputLevelPollCallback{ nullptr };
    std::function<void(MemaInputCommander* sender, int, bool)>  m_inputMuteChangeCallback{ nullptr };
    std::function<void(MemaInputCommander* sender, int)>        m_inputMutePollCallback{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaInputCommander)
};

class MemaOutputCommander : public MemaChannelCommander
{
public:
    MemaOutputCommander();
    virtual ~MemaOutputCommander() override;

    void setOutputMuteChangeCallback(const std::function<void(MemaOutputCommander* sender, int, bool)>& callback);
    void setOutputLevelChangeCallback(const std::function<void(MemaOutputCommander* sender, int, float)>& callback);
    void setOutputMutePollCallback(const std::function<void(MemaOutputCommander* sender, int)>& callback);
    void setOutputLevelPollCallback(const std::function<void(MemaOutputCommander* sender, int)>& callback);

    virtual void setOutputMute(unsigned int channel, bool muteState) = 0;
    virtual void setOutputLevel(unsigned int channel, float levelValue) { ignoreUnused(channel); ignoreUnused(levelValue); };

protected:
    void outputMuteChange(int channel, bool muteState);
    void outputLevelChange(int channel, float levelValue);
        
    void outputMutePoll(int channel);
    void outputLevelPoll(int channel);

private:
    std::function<void(MemaOutputCommander* sender, int, float)>    m_outputLevelChangeCallback{ nullptr };
    std::function<void(MemaOutputCommander* sender, int)>           m_outputLevelPollCallback{ nullptr };
    std::function<void(MemaOutputCommander* sender, int, bool)>     m_outputMuteChangeCallback{ nullptr };
    std::function<void(MemaOutputCommander* sender, int)>           m_outputMutePollCallback{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaOutputCommander)
};
    
class MemaCrosspointCommander : public MemaChannelCommander
{
public:
    MemaCrosspointCommander();
    virtual ~MemaCrosspointCommander() override;

    void setCrosspointEnabledChangeCallback(const std::function<void(MemaCrosspointCommander* sender, int, int, bool)>& callback);
    void setCrosspointEnabledPollCallback(const std::function<void(MemaCrosspointCommander* sender, int, int)>& callback);
    virtual void setCrosspointEnabledValue(int input, int output, bool enabledState) = 0;

    void setCrosspointFactorChangeCallback(const std::function<void(MemaCrosspointCommander* sender, int, int, float)>& callback);
    void setCrosspointFactorPollCallback(const std::function<void(MemaCrosspointCommander* sender, int, int)>& callback);
    virtual void setCrosspointFactorValue(int input, int output, float factor) = 0;

    virtual void setIOCount(int inputCount, int outputCount) = 0;

protected:
    void crosspointEnabledChange(int input, int output, bool enabledState);
    void crosspointEnabledPoll(int input, int output);

    void crosspointFactorChange(int input, int output, float factor);
    void crosspointFactorPoll(int input, int output);

private:
    void setChannelCount(int channelCount) override { ignoreUnused(channelCount); jassertfalse; };

    std::function<void(MemaCrosspointCommander* sender, int, int, bool)> m_crosspointEnabledChangeCallback{ nullptr };
    std::function<void(MemaCrosspointCommander* sender, int, int)>       m_crosspointEnabledPollCallback{ nullptr };

    std::function<void(MemaCrosspointCommander* sender, int, int, float)>   m_crosspointFactorChangeCallback{ nullptr };
    std::function<void(MemaCrosspointCommander* sender, int, int)>          m_crosspointFactorPollCallback{ nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MemaCrosspointCommander)
};


} // namespace Mema

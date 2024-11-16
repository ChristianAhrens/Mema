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

#include "MemaCommanders.h"


namespace Mema
{


MemaChannelCommander::MemaChannelCommander()
{
}

MemaChannelCommander::~MemaChannelCommander()
{
}

MemaInputCommander::MemaInputCommander()
{
}

MemaInputCommander::~MemaInputCommander()
{
}

void MemaInputCommander::setInputMuteChangeCallback(const std::function<void(MemaInputCommander* sender, int, bool)>& callback)
{
	m_inputMuteChangeCallback = callback;
}


void MemaInputCommander::setInputLevelChangeCallback(const std::function<void(MemaInputCommander* sender, int, float)>& callback)
{
	m_inputLevelChangeCallback = callback;
}
void MemaInputCommander::setInputMutePollCallback(const std::function<void(MemaInputCommander* sender, int)>& callback)
{
	m_inputMutePollCallback = callback;
}


void MemaInputCommander::setInputLevelPollCallback(const std::function<void(MemaInputCommander* sender, int)>& callback)
{
	m_inputLevelPollCallback = callback;
}
void MemaInputCommander::inputMuteChange(int channel, bool muteState)
{
	if (m_inputMuteChangeCallback)
		m_inputMuteChangeCallback(this, channel, muteState);
}


void MemaInputCommander::inputLevelChange(int channel, float levelValue)
{
	if (m_inputLevelChangeCallback)
		m_inputLevelChangeCallback(this, channel, levelValue);
}
void MemaInputCommander::inputMutePoll(int channel)
{
	if (m_inputMutePollCallback)
		m_inputMutePollCallback(this, channel);
}


void MemaInputCommander::inputLevelPoll(int channel)
{
	if (m_inputLevelPollCallback)
		m_inputLevelPollCallback(this, channel);
}


MemaOutputCommander::MemaOutputCommander()
{
}

MemaOutputCommander::~MemaOutputCommander()
{
}

void MemaOutputCommander::setOutputMuteChangeCallback(const std::function<void(MemaOutputCommander* sender, int, bool)>& callback)
{
	m_outputMuteChangeCallback = callback;
}

void MemaOutputCommander::setOutputLevelChangeCallback(const std::function<void(MemaOutputCommander* sender, int, float)>& callback)
{
	m_outputLevelChangeCallback = callback;
}
void MemaOutputCommander::setOutputMutePollCallback(const std::function<void(MemaOutputCommander* sender, int)>& callback)
{
	m_outputMutePollCallback = callback;
}

void MemaOutputCommander::setOutputLevelPollCallback(const std::function<void(MemaOutputCommander* sender, int)>& callback)
{
	m_outputLevelPollCallback = callback;
}

void MemaOutputCommander::outputMuteChange(int channel, bool muteState)
{
	if (m_outputMuteChangeCallback)
		m_outputMuteChangeCallback(nullptr, channel, muteState);
}

void MemaOutputCommander::outputLevelChange(int channel, float levelValue)
{
	if (m_outputLevelChangeCallback)
		m_outputLevelChangeCallback(nullptr, channel, levelValue);
}

void MemaOutputCommander::outputMutePoll(int channel)
{
	if (m_outputMutePollCallback)
		m_outputMutePollCallback(nullptr, channel);
}

void MemaOutputCommander::outputLevelPoll(int channel)
{
	if (m_outputLevelPollCallback)
		m_outputLevelPollCallback(nullptr, channel);
}


MemaCrosspointCommander::MemaCrosspointCommander()
{
}

MemaCrosspointCommander::~MemaCrosspointCommander()
{
}

void MemaCrosspointCommander::setCrosspointEnabledChangeCallback(const std::function<void(MemaCrosspointCommander* sender, int, int, bool)>& callback)
{
	m_crosspointEnabledChangeCallback = callback;
}

void MemaCrosspointCommander::setCrosspointEnabledPollCallback(const std::function<void(MemaCrosspointCommander* sender, int, int)>& callback)
{
	m_crosspointEnabledPollCallback = callback;
}

void MemaCrosspointCommander::crosspointEnabledChange(int input, int output, bool enabledState)
{
	if (m_crosspointEnabledChangeCallback)
		m_crosspointEnabledChangeCallback(nullptr, input, output, enabledState);
}

void MemaCrosspointCommander::crosspointEnabledPoll(int input, int output)
{
	if (m_crosspointEnabledPollCallback)
		m_crosspointEnabledPollCallback(nullptr, input, output);
}


} // namespace Mema

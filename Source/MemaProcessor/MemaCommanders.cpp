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

void MemaInputCommander::setInputMuteChangeCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t, bool)>& callback)
{
	m_inputMuteChangeCallback = callback;
}


void MemaInputCommander::setInputLevelChangeCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t, float)>& callback)
{
	m_inputLevelChangeCallback = callback;
}
void MemaInputCommander::setInputMutePollCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t)>& callback)
{
	m_inputMutePollCallback = callback;
}


void MemaInputCommander::setInputLevelPollCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t)>& callback)
{
	m_inputLevelPollCallback = callback;
}
void MemaInputCommander::inputMuteChange(std::uint16_t channel, bool muteState)
{
	if (m_inputMuteChangeCallback)
		m_inputMuteChangeCallback(this, channel, muteState);
}


void MemaInputCommander::inputLevelChange(std::uint16_t channel, float levelValue)
{
	if (m_inputLevelChangeCallback)
		m_inputLevelChangeCallback(this, channel, levelValue);
}
void MemaInputCommander::inputMutePoll(std::uint16_t channel)
{
	if (m_inputMutePollCallback)
		m_inputMutePollCallback(this, channel);
}


void MemaInputCommander::inputLevelPoll(std::uint16_t channel)
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

void MemaOutputCommander::setOutputMuteChangeCallback(const std::function<void(MemaOutputCommander* sender, std::uint16_t, bool)>& callback)
{
	m_outputMuteChangeCallback = callback;
}

void MemaOutputCommander::setOutputLevelChangeCallback(const std::function<void(MemaOutputCommander* sender, std::uint16_t, float)>& callback)
{
	m_outputLevelChangeCallback = callback;
}
void MemaOutputCommander::setOutputMutePollCallback(const std::function<void(MemaOutputCommander* sender, std::uint16_t)>& callback)
{
	m_outputMutePollCallback = callback;
}

void MemaOutputCommander::setOutputLevelPollCallback(const std::function<void(MemaOutputCommander* sender, std::uint16_t)>& callback)
{
	m_outputLevelPollCallback = callback;
}

void MemaOutputCommander::outputMuteChange(std::uint16_t channel, bool muteState)
{
	if (m_outputMuteChangeCallback)
		m_outputMuteChangeCallback(nullptr, channel, muteState);
}

void MemaOutputCommander::outputLevelChange(std::uint16_t channel, float levelValue)
{
	if (m_outputLevelChangeCallback)
		m_outputLevelChangeCallback(nullptr, channel, levelValue);
}

void MemaOutputCommander::outputMutePoll(std::uint16_t channel)
{
	if (m_outputMutePollCallback)
		m_outputMutePollCallback(nullptr, channel);
}

void MemaOutputCommander::outputLevelPoll(std::uint16_t channel)
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

void MemaCrosspointCommander::setCrosspointEnabledChangeCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t, bool)>& callback)
{
	m_crosspointEnabledChangeCallback = callback;
}

void MemaCrosspointCommander::setCrosspointEnabledPollCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t)>& callback)
{
	m_crosspointEnabledPollCallback = callback;
}

void MemaCrosspointCommander::crosspointEnabledChange(std::uint16_t input, std::uint16_t output, bool enabledState)
{
	if (m_crosspointEnabledChangeCallback)
		m_crosspointEnabledChangeCallback(nullptr, input, output, enabledState);
}

void MemaCrosspointCommander::crosspointEnabledPoll(std::uint16_t input, std::uint16_t output)
{
	if (m_crosspointEnabledPollCallback)
		m_crosspointEnabledPollCallback(nullptr, input, output);
}

void MemaCrosspointCommander::setCrosspointFactorChangeCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t, float)>& callback)
{
	m_crosspointFactorChangeCallback = callback;
}

void MemaCrosspointCommander::setCrosspointFactorPollCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t)>& callback)
{
	m_crosspointFactorPollCallback = callback;
}

void MemaCrosspointCommander::crosspointFactorChange(std::uint16_t input, std::uint16_t output, float factor)
{
	if (m_crosspointFactorChangeCallback)
		m_crosspointFactorChangeCallback(nullptr, input, output, factor);
}

void MemaCrosspointCommander::crosspointFactorPoll(std::uint16_t input, std::uint16_t output)
{
	if (m_crosspointFactorPollCallback)
		m_crosspointFactorPollCallback(nullptr, input, output);
}


} // namespace Mema

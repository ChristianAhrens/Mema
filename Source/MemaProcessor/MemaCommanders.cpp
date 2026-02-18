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
	onInputMuteChangeCallback = callback;
}

void MemaInputCommander::setInputLevelChangeCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t, float)>& callback)
{
	onInputLevelChangeCallback = callback;
}

void MemaInputCommander::setInputMutePollCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t)>& callback)
{
	onInputMutePollCallback = callback;
}

void MemaInputCommander::setInputLevelPollCallback(const std::function<void(MemaInputCommander* sender, std::uint16_t)>& callback)
{
	onInputLevelPollCallback = callback;
}

void MemaInputCommander::inputMuteChange(std::uint16_t channel, bool muteState, MemaInputCommander* /*sender*/)
{
	if (onInputMuteChangeCallback)
		onInputMuteChangeCallback(this, channel, muteState);
}

void MemaInputCommander::inputLevelChange(std::uint16_t channel, float levelValue, MemaInputCommander* /*sender*/)
{
	if (onInputLevelChangeCallback)
		onInputLevelChangeCallback(this, channel, levelValue);
}

void MemaInputCommander::inputMutePoll(std::uint16_t channel, MemaInputCommander* /*sender*/)
{
	if (onInputMutePollCallback)
		onInputMutePollCallback(this, channel);
}

void MemaInputCommander::inputLevelPoll(std::uint16_t channel, MemaInputCommander* /*sender*/)
{
	if (onInputLevelPollCallback)
		onInputLevelPollCallback(this, channel);
}


MemaOutputCommander::MemaOutputCommander()
{
}

MemaOutputCommander::~MemaOutputCommander()
{
}

void MemaOutputCommander::setOutputMuteChangeCallback(const std::function<void(MemaOutputCommander* /*sender*/, std::uint16_t, bool)>& callback)
{
	onOutputMuteChangeCallback = callback;
}

void MemaOutputCommander::setOutputLevelChangeCallback(const std::function<void(MemaOutputCommander* /*sender*/, std::uint16_t, float)>& callback)
{
	onOutputLevelChangeCallback = callback;
}

void MemaOutputCommander::setOutputMutePollCallback(const std::function<void(MemaOutputCommander* /*sender*/, std::uint16_t)>& callback)
{
	onOutputMutePollCallback = callback;
}

void MemaOutputCommander::setOutputLevelPollCallback(const std::function<void(MemaOutputCommander* /*sender*/, std::uint16_t)>& callback)
{
	onOutputLevelPollCallback = callback;
}

void MemaOutputCommander::outputMuteChange(std::uint16_t channel, bool muteState, MemaOutputCommander* /*sender*/)
{
	if (onOutputMuteChangeCallback)
		onOutputMuteChangeCallback(nullptr, channel, muteState);
}

void MemaOutputCommander::outputLevelChange(std::uint16_t channel, float levelValue, MemaOutputCommander* /*sender*/)
{
	if (onOutputLevelChangeCallback)
		onOutputLevelChangeCallback(nullptr, channel, levelValue);
}

void MemaOutputCommander::outputMutePoll(std::uint16_t channel, MemaOutputCommander* /*sender*/)
{
	if (onOutputMutePollCallback)
		onOutputMutePollCallback(nullptr, channel);
}

void MemaOutputCommander::outputLevelPoll(std::uint16_t channel, MemaOutputCommander* /*sender*/)
{
	if (onOutputLevelPollCallback)
		onOutputLevelPollCallback(nullptr, channel);
}


MemaCrosspointCommander::MemaCrosspointCommander()
{
}

MemaCrosspointCommander::~MemaCrosspointCommander()
{
}

void MemaCrosspointCommander::setCrosspointEnabledChangeCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t, bool)>& callback)
{
	onCrosspointEnabledChangeCallback = callback;
}

void MemaCrosspointCommander::setCrosspointEnabledPollCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t)>& callback)
{
	onCrosspointEnabledPollCallback = callback;
}

void MemaCrosspointCommander::crosspointEnabledChange(std::uint16_t input, std::uint16_t output, bool enabledState, MemaCrosspointCommander* sender)
{
	if (onCrosspointEnabledChangeCallback)
		onCrosspointEnabledChangeCallback(sender, input, output, enabledState);
}

void MemaCrosspointCommander::crosspointEnabledPoll(std::uint16_t input, std::uint16_t output, MemaCrosspointCommander* sender)
{
	if (onCrosspointEnabledPollCallback)
		onCrosspointEnabledPollCallback(sender, input, output);
}

void MemaCrosspointCommander::setCrosspointFactorChangeCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t, float)>& callback)
{
	onCrosspointFactorChangeCallback = callback;
}

void MemaCrosspointCommander::setCrosspointFactorPollCallback(const std::function<void(MemaCrosspointCommander* sender, std::uint16_t, std::uint16_t)>& callback)
{
	onCrosspointFactorPollCallback = callback;
}

void MemaCrosspointCommander::crosspointFactorChange(std::uint16_t input, std::uint16_t output, float factor, MemaCrosspointCommander* sender)
{
	if (onCrosspointFactorChangeCallback)
		onCrosspointFactorChangeCallback(sender, input, output, factor);
}

void MemaCrosspointCommander::crosspointFactorPoll(std::uint16_t input, std::uint16_t output, MemaCrosspointCommander* sender)
{
	if (onCrosspointFactorPollCallback)
		onCrosspointFactorPollCallback(sender, input, output);
}


MemaPluginCommander::MemaPluginCommander()
{

}

MemaPluginCommander::~MemaPluginCommander()
{

}

void MemaPluginCommander::setPluginParameterInfosChangeCallback(const std::function<void(MemaPluginCommander* sender, const std::vector<PluginParameterInfo>&, const std::string&)>& callback)
{
	onPluginParameterInfosChangeCallback = callback;
}

void MemaPluginCommander::setPluginParameterInfosPollCallback(const std::function<void(MemaPluginCommander* sender)>& callback)
{
	onPluginParameterInfosPollCallback = callback;
}

void MemaPluginCommander::pluginParameterInfosChange(const std::vector<PluginParameterInfo>& infos, const std::string& name, MemaPluginCommander* sender)
{
	if (onPluginParameterInfosChangeCallback)
		onPluginParameterInfosChangeCallback(sender, infos, name);
}

void MemaPluginCommander::pluginParameterInfosPoll(MemaPluginCommander* sender)
{
	if (onPluginParameterInfosPollCallback)
		onPluginParameterInfosPollCallback(sender);
}

void MemaPluginCommander::setPluginParameterValueChangeCallback(const std::function<void(MemaPluginCommander* sender, std::uint16_t, std::string, float)>& callback)
{
	onPluginParameterValueChangeCallback = callback;
}

void MemaPluginCommander::setPluginParameterValuePollCallback(const std::function<void(MemaPluginCommander* sender, std::uint16_t, std::string)>& callback)
{
	onPluginParameterValuePollCallback = callback;
}

void MemaPluginCommander::pluginParameterValueChange(std::uint16_t index, std::string id, float currentValue, MemaPluginCommander* sender)
{
	if (onPluginParameterValueChangeCallback)
		onPluginParameterValueChangeCallback(sender, index, id, currentValue);
}

void MemaPluginCommander::pluginParameterValuePoll(std::uint16_t index, std::string id, MemaPluginCommander * sender)
{
	if (onPluginParameterValuePollCallback)
		onPluginParameterValuePollCallback(sender, index, id);
}


} // namespace Mema

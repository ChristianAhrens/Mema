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


//==============================================================================
class InterprocessConnectionImpl : public juce::InterprocessConnection
{
public:
    InterprocessConnectionImpl(int id);
    virtual ~InterprocessConnectionImpl();

    void connectionMade() override;

    void connectionLost() override;

    void messageReceived(const MemoryBlock& message) override;

    int getId();

    std::function<void(int)>                        onConnectionMade;
    std::function<void(int)>                        onConnectionLost;
    std::function<void(int, const MemoryBlock&)>    onMessageReceived;

private:
    int m_id;

};

//==============================================================================
class InterprocessConnectionServerImpl : public juce::InterprocessConnectionServer
{
public:
    InterprocessConnectionServerImpl();
    virtual ~InterprocessConnectionServerImpl();

    void createMessageThread(int id);

    std::map<int, std::pair<double, bool>> getListHealth();

    bool hasActiveConnection(int id);
    bool hasActiveConnections();
    const std::unique_ptr<InterprocessConnectionImpl>& getActiveConnection(int id);
    const std::vector<int> cleanupDeadConnections();

    bool enqueueMessage(const MemoryBlock& message, std::vector<int> sendIds = {});

    std::function<void(int)>   onConnectionCreated;

    static constexpr double s_listSizeThreshold = 35.0;

private:
    InterprocessConnection* createConnectionObject();
    void endMessageThread(int id);

    std::map<int, std::mutex>                       m_sendMessageMutexs;
    std::map<int, std::queue<juce::MemoryBlock>>    m_sendMessageLists;
    std::map<int, bool>                             m_sendMessageListClipped;
    std::map<int, std::atomic<bool>>                m_sendMessageResults;

    std::map<int, std::atomic<bool>>            m_sendMessageThreadsActive;
    std::map<int, std::unique_ptr<std::thread>> m_sendMessageThreads;
    std::map<int, std::condition_variable>		m_sendMessageCVs;
    std::map<int, std::mutex>                   m_sendMessageCVMutexs;

    std::map<int, std::unique_ptr<InterprocessConnectionImpl>> m_connections;
    int m_connectionIdIter = 0;
};


}; // namespace Mema

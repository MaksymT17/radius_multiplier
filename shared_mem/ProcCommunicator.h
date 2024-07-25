#pragma once

#include <memory>
#include "SharedMemorySender.h"
#include "SharedMemoryReceiver.h"
#include "Message.hpp"

class ProcCommunicator
{
public:
    ProcCommunicator(bool isMasterMode, const std::string& shMemName);
    ~ProcCommunicator();

    void send(const Message *msg);
    Message *receive();

private:
    std::unique_ptr<SharedMemorySender> m_sender;
    std::unique_ptr<SharedMemoryReceiver> m_receiver;
    bool m_master_mode;
    sem_t *m_master_received;
    sem_t *m_slave_received;
    sem_t *m_master_sent;
    sem_t *m_slave_sent;
};
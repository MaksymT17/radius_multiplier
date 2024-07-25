#include "ProcCommunicator.h"
#include <iostream>
#include <unistd.h>

#define INITIAL_VALUE 0

ProcCommunicator::ProcCommunicator(bool isMasterMode, const std::string &shMemName) : m_master_mode(isMasterMode),
                                                                                      m_master_received((sem_t *)-1),
                                                                                      m_slave_received((sem_t *)-1),
                                                                                      m_master_sent((sem_t *)-1),
                                                                                      m_slave_sent((sem_t *)-1)
{
    const std::string master_mem_name = shMemName + "_master";
    const std::string slave_mem_name = shMemName + "_slave";
    if (isMasterMode)
    {
        m_sender = std::make_unique<SharedMemorySender>(master_mem_name.c_str());
        m_receiver = std::make_unique<SharedMemoryReceiver>(slave_mem_name.c_str());

        m_master_received = sem_open("/master_rsem", O_CREAT, 0666, INITIAL_VALUE);
        m_slave_received = sem_open("/slave_rsem", O_CREAT, 0666, INITIAL_VALUE);
        m_master_sent = sem_open("/master_sem", O_CREAT, 0666, INITIAL_VALUE);
        m_slave_sent = sem_open("/slave_sem", O_CREAT, 0666, INITIAL_VALUE);

        if (m_master_received == SEM_FAILED || m_slave_received == SEM_FAILED || m_master_sent == SEM_FAILED || m_slave_sent == SEM_FAILED)
        {
            perror("SharedMemorySender sem_open");
        }
    }
    else
    {
        m_sender = std::make_unique<SharedMemorySender>(slave_mem_name.c_str());
        m_receiver = std::make_unique<SharedMemoryReceiver>(master_mem_name.c_str());

        while (m_master_received == SEM_FAILED || m_slave_received == SEM_FAILED || m_master_sent == SEM_FAILED || m_slave_sent == SEM_FAILED)
        {
            std::cout << "slave is not ready, attempting" << std::endl;
            m_master_received = sem_open("/master_rsem", O_RDWR, 0666, INITIAL_VALUE);
            m_slave_received = sem_open("/slave_rsem", O_RDWR, 0666, INITIAL_VALUE);
            m_master_sent = sem_open("/master_sem", O_RDWR, 0666, INITIAL_VALUE);
            m_slave_sent = sem_open("/slave_sem", O_RDWR, 0666, INITIAL_VALUE);
            sleep(1);
        }
    }
}
ProcCommunicator::~ProcCommunicator()
{
    if (sem_close(m_master_received) == -1)
    {
        perror("Failed to destroy m_master_received semaphore");
    }
    if (sem_close(m_slave_received) == -1)
    {
        perror("Failed to destroy m_slave_received semaphore");
    }
    if (sem_close(m_master_sent) == -1)
    {
        perror("Failed to destroy m_master_sent semaphore");
    }
    if (sem_close(m_slave_sent) == -1)
    {
        perror("Failed to destroy m_slave_sent semaphore");
    }
}

void ProcCommunicator::send(const Message *msg)
{
    m_sender->sendMessage(msg);
    sem_post(m_master_mode ? m_master_sent : m_slave_sent);
    sem_wait(m_master_mode ? m_slave_received : m_master_received);
}

Message *ProcCommunicator::receive()
{
    sem_wait(m_master_mode ? m_slave_sent : m_master_sent);
    Message *response = m_receiver->receiveMessage();
    sem_post(m_master_mode ? m_master_received : m_slave_received);
    return response;
}

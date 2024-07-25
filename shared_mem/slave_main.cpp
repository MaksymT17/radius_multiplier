#include "ProcCommunicator.h"
#include <iostream>
#include <thread>
#include <chrono>

static const std::string shared_mem_name{"/shared_mem"};

int main()
{
    std::cout << "Background task started...\n";
    ProcCommunicator *slave = new ProcCommunicator(false, shared_mem_name);

    auto res = slave->receive();
    std::vector<int> vec1{1, 2, 3, 4, 5, 6};
    Message msg{res->id + 1, MessageType::HANDSHAKE_OK, nullptr, vec1};
    slave->send(&msg);

    auto res2 = slave->receive();
    // std::vector<int> vec1{1, 2, 3, 4, 5, 6};
    Message msg2{res2->id + 1, MessageType::SET_CONFIG_OK, nullptr, vec1};
    slave->send(&msg2);

    std::this_thread::sleep_for(std::chrono::seconds(20));
    delete slave;
    std::cout << "Background task completed.\n";

    return 0;
}
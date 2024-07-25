#include "ProcCommunicator.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

static const std::string shared_mem_name{"/shared_mem"};

int main()
{
    std::cout << "Main thread starts...\n";
    std::vector<int> vec1{1, 2, 3, 4, 5};

    Message msg{1, MessageType::HANDSHAKE, nullptr, vec1};
    ProcCommunicator master(true, shared_mem_name);
    // Create a thread and execute the backgroundTask function
    //std::thread worker(backgroundTask);

    // std::this_thread::sleep_for(std::chrono::seconds(5));
    master.send(&msg);
    auto msg_resp = master.receive();

    std::this_thread::sleep_for(std::chrono::seconds(5));
    Message msg1{3, MessageType::SET_CONFIG, nullptr, vec1};
    master.send(&msg1);
    msg_resp = master.receive();

    //worker.join();

    std::cout << "Main thread ends.\n";

    return 0;
}
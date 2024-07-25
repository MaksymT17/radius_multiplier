#include "ProcCommunicator.h"
#include <iostream>
#include <thread>
#include <chrono>

static const std::string shared_mem_name{"/shared_mem"};

// Function to be executed in a separate thread
void backgroundTask()
{
    std::cout << "Background task started...\n";
    ProcCommunicator *slave = new ProcCommunicator(false,shared_mem_name);

    auto res = slave->receive();
    std::vector<int> vec1{1, 2, 3, 4, 5, 6};
    Message msg{res->id + 1, MessageType::HANDSHAKE_OK};
    slave->send(&msg);

    auto res2 = slave->receive(); // depending on type payload can be extracted
    if (res2->type == MessageType::SET_CONFIG)
    {
        MessageSetConfig *setc = static_cast<MessageSetConfig *>(res2);
        printf("%d\n", setc->conf);
    }
    Message msg2{res2->id + 1, MessageType::SET_CONFIG_OK};
    slave->send(&msg2);

    auto res3 = slave->receive(); // depending on type payload can be extracted
    if (res3->type == MessageType::COMPARE_REQUEST)
    {
        MessageCompareRequest *setc = static_cast<MessageCompareRequest *>(res2);
        printf("%d\n", setc->f1);
    }

    MessageCompareResult msg3{res3->id + 1, MessageType::COMPARE_RESULT, 789};
    slave->send(&msg3);

    //std::this_thread::sleep_for(std::chrono::seconds(10));
    delete slave;
    std::cout << "Background task completed.\n";
}

int main()
{
    std::cout << "Main thread starts...\n";
    std::vector<int> vec1{1, 2, 3, 4, 5};

    Message msg_hand{1, MessageType::HANDSHAKE};
    ProcCommunicator master(true, shared_mem_name);
    // Create a thread and execute the backgroundTask function
    std::thread worker(backgroundTask);

    // std::this_thread::sleep_for(std::chrono::seconds(5));
    master.send(&msg_hand);
    auto msg_resp = master.receive();

    // std::this_thread::sleep_for(std::chrono::seconds(5));
    MessageSetConfig msg_set_conf{3, MessageType::SET_CONFIG, 17};
    master.send(&msg_set_conf);
    msg_resp = master.receive();

    MessageCompareRequest msg2{5, MessageType::COMPARE_REQUEST, 123, 456};
    master.send(&msg2);
    msg_resp = master.receive();

    if (msg_resp->type == MessageType::COMPARE_RESULT)
    {
        MessageCompareResult *setc = static_cast<MessageCompareResult *>(msg_resp);
        std::cout << "COMPARE_RESULT received.\n";
    }

    worker.join();

    

    return 0;
}
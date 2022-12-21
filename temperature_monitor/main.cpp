#include "TempDevicesMonitor.hpp"
#include <memory>

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

// g++ main.cpp Types.hpp TempDevicesMonitor.hpp -o temp_monitor

std::unique_ptr<TempDevicesMonitor> monitor;

void handler(int signo)
{
    monitor->stop();
}

int main()
{
    struct sigaction sigbreak;
    sigbreak.sa_handler = &handler;
    sigemptyset(&sigbreak.sa_mask);
    sigbreak.sa_flags = 0;
    if (sigaction(SIGINT, &sigbreak, NULL) != 0)
        std::perror("sigaction");

    TempDeviceLimits b({10, 55}, {-10, 80}), m({0, 80}, {-20, 85});
    monitor = std::unique_ptr<TempDevicesMonitor>(new TempDevicesMonitor(b, m));
    monitor->start();

    return 0;
}

#include <atomic>
#include <fstream>
#include <chrono>
#include <thread>
#include "Types.hpp"

constexpr size_t DEV_CHECK_PERIOD_SEC = 5;
constexpr size_t OUTSIDE_TO_DEV_PERIOD_RATIO = 5;

constexpr const char *motor_temp_path = "dev/motor";
constexpr const char *battery_temp_path = "dev/battery";
constexpr const char *outside_temp_path = "dev/outside";

class TempDevicesMonitor
{
public:
    TempDevicesMonitor() = delete;
    TempDevicesMonitor(const TempDeviceLimits &aBat, const TempDeviceLimits &aMot) : battery(aBat), motor(aMot), running(false) {}

    void start()
    {
        if (running)
        {
            std::cout << "ERROR: TempDevicesMonitor already running" << std::endl;
            return;
        }
        running = true;

        std::ifstream motor_indicator(motor_temp_path);
        std::ifstream battery_indicator(battery_temp_path);
        std::ifstream outside_indicator(outside_temp_path);

        std::string motor_temp, battery_temp, outside_temp;

        if (!outside_indicator.is_open() && !std::getline(outside_indicator, outside_temp))
        {
            std::cout << "ERROR: Read outside temp failed" << std::endl;
            running = false;
            return;
        }

        int outside_temp_num = std::atoi(outside_temp.c_str());
        int dev_check_iteration = 0;

        // In this demo reading of file has shown,
        // in real world, each device can have own implementation to get temperature indications
        // e.g. app(user space) -> kernel -> HAL device
        while (running)
        {
            // measure battery & motor temperature more often than outside temp(changes not so fast, typically).
            if (motor_indicator.is_open() && std::getline(motor_indicator, motor_temp))
            {
                motor.indication(std::atoi(motor_temp.c_str()), outside_temp_num);
            }
            if (battery_indicator.is_open() && std::getline(battery_indicator, battery_temp))
            {
                battery.indication(std::atoi(battery_temp.c_str()), outside_temp_num);
            }

            dev_check_iteration++;
            if (dev_check_iteration == OUTSIDE_TO_DEV_PERIOD_RATIO)
            {
                dev_check_iteration = 0;
                if (outside_indicator.is_open() && std::getline(outside_indicator, outside_temp))
                {
                    outside_temp_num = std::atoi(outside_temp.c_str());
                    std::cout << "outside temp:" << outside_temp << std::endl;
                }
            }
            // reset file read
            motor_indicator.clear();
            motor_indicator.seekg(0);
            battery_indicator.clear();
            battery_indicator.seekg(0);
            outside_indicator.clear();
            outside_indicator.seekg(0);

            std::this_thread::sleep_for(std::chrono::seconds(DEV_CHECK_PERIOD_SEC));
        }
        std::cout << "TempDevicesMonitor finished monitoring" << std::endl;
    }

    void stop()
    {
        std::cout << "TempDevicesMonitor stopping..." << std::endl;
        running = false;
    }

private:
    TempDeviceLimits battery;
    TempDeviceLimits motor;
    std::atomic<bool> running;
};
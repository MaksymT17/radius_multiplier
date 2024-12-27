#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

struct Lap {
    float time;
    bool dirty_air;
    bool out_lap;

    Lap(float t, bool dirty, bool out) : time(t), dirty_air(dirty), out_lap(out) {}
};

struct Driver {
    int number;
    std::string name;
    int team_id;
    int position;
    std::vector<Lap> laps;
    int tire_type; // 0: SOFT, 1: MEDIUM, 2: HARD, 3: INTER, 4: WET
    int tire_age;

    Driver(int num, const std::string &n, int team, int tire_type)
        : number(num), name(n), team_id(team), position(0), tire_type(tire_type), tire_age(0) {}

    Driver() : number(0), name("Unknown"), team_id(0), position(0), tire_type(0), tire_age(0) {}

    void addLap(float time, bool dirty_air, bool out_lap) {
        laps.emplace_back(time, dirty_air, out_lap);
        tire_age += !out_lap; // Increment tire age only if not an out lap
    }
};

class RaceManager {
private:
    std::unordered_map<int, Driver> drivers; // Map driver number to Driver object
    std::mutex data_mutex;
    std::condition_variable data_condition;
    bool running;

    static const std::unordered_map<int, int> tire_lifetimes; // Tire lifetime mapping

public:
    RaceManager() : running(true) {}

    void addDriver(int number, const std::string &name, int team_id, int tire_type) {
        std::lock_guard<std::mutex> lock(data_mutex);
        drivers[number] = Driver(number, name, team_id, tire_type);
    }

    void addLapData(int driver_number, float time, bool dirty_air, bool out_lap) {
        std::lock_guard<std::mutex> lock(data_mutex);
        if (drivers.find(driver_number) != drivers.end()) {
            drivers[driver_number].addLap(time, dirty_air, out_lap);
        }
    }

    float makeLapTimeForecast(int driver_number, size_t history_laps = 5) {
        std::lock_guard<std::mutex> lock(data_mutex);

        if (drivers.find(driver_number) == drivers.end() || drivers[driver_number].laps.empty()) {
            return 0.0f; // No data available
        }

        Driver &driver = drivers[driver_number];
        const auto &laps = driver.laps;
        int tire_type = driver.tire_type;
        int tire_age = driver.tire_age;

        size_t start_index = laps.size() >= history_laps ? laps.size() - history_laps : 0;
        float total_time = 0.0f;
        size_t count = 0;

        for (size_t i = start_index; i < laps.size(); ++i) {
            const Lap &lap = laps[i];

            if (lap.out_lap || (i > 0 && std::abs(lap.time - laps[i - 1].time) > 2.0f)) {
                continue; // Skip out laps and unusual laps
            }

            total_time += lap.time;
            count++;
        }

        if (count == 0) {
            return 0.0f; // No valid laps to forecast
        }

        float average_time = total_time / count;

        // Adjust for tire degradation
        if (tire_lifetimes.find(tire_type) != tire_lifetimes.end()) {
            int lifetime = tire_lifetimes.at(tire_type);
            if (tire_age > lifetime) {
                average_time += (tire_age - lifetime) * 0.230f;
            }
        }

        // Check for improving trend
        if (laps.size() > 1) {
            const Lap &last_lap = laps.back();
            const Lap &second_last_lap = laps[laps.size() - 2];

            if (!last_lap.dirty_air && last_lap.time < second_last_lap.time) {
                average_time = (last_lap.time + second_last_lap.time) / 2.0f;
            }
        }

        return std::max(average_time, 0.0f); // Ensure forecast is non-negative
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(data_mutex);
            running = false;
        }
        data_condition.notify_all();
    }

    void dataFetcher() {
        while (running) {
            std::unique_lock<std::mutex> lock(data_mutex);
            data_condition.wait(lock, [this]() { return !running; });

            // Simulate data fetching (placeholder)
            // In a real scenario, fetch data from a file, network, or telemetry source
        }
    }
};

const std::unordered_map<int, int> RaceManager::tire_lifetimes = {
    // example of tire lifespan without significant degradation.
    // Dependent on weather, temperature, track etc.
    // Can be collected during the tresting stage with similar to race conditions.
    {0, 10}, // SOFT
    {1, 15}, // MEDIUM
    {2, 20}, // HARD
    {3, 20}, // INTER
    {4, 25}  // WET
};

int main() {
    RaceManager manager;

    manager.addDriver(1, "Max ", 1, 0);
    manager.addDriver(44, "Lewis ", 2, 1);

    manager.addLapData(1, 78.5, false, false);
    manager.addLapData(1, 78.8, true, false);
    manager.addLapData(1, 79.2, false, true);
    manager.addLapData(1, 78.3, false, false);
    manager.addLapData(1, 81.0, false, false);

    manager.addLapData(1, 78.5, false, false);
    manager.addLapData(1, 78.8, true, false);
    manager.addLapData(1, 79.2, false, true);
    manager.addLapData(1, 78.3, false, false);
    manager.addLapData(1, 81.0, false, false);

    manager.addLapData(1, 78.5, false, false);
    manager.addLapData(1, 78.8, true, false);
    manager.addLapData(1, 79.2, false, true);
    manager.addLapData(1, 78.3, false, false);
    manager.addLapData(1, 81.0, false, false);


    float forecast = manager.makeLapTimeForecast(1);
    std::cout << "Lap time forecast for Max : " << forecast << " seconds" << std::endl;

    return 0;
}

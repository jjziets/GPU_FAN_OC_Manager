#include <iostream>
#include <nvml.h>
#include <cstdlib>
#include <thread>
#include <chrono>

#define MAX_FAN_SPEED 100
#define AUTO_FAN_SPEED 0

// Function to get the appropriate fan speed based on current GPU temperature
unsigned int getFanSpeed(int current_temp, int max_temp) {
    int difference = max_temp - current_temp;
    if (difference >= 40) return AUTO_FAN_SPEED;
    if (difference >= 35) return 30;
    if (difference >= 30) return 40;
    if (difference >= 25) return 50;
    if (difference >= 20) return 60;
    if (difference >= 15) return 70;
    if (difference >= 10) return 80;
    if (difference >= 5) return 90;
    return MAX_FAN_SPEED;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./set_fan_curve <max_temperature>" << std::endl;
        return 1;
    }

    int max_temp = std::atoi(argv[1]);
    if (max_temp < 50 || max_temp > 90) {
        std::cerr << "Max temperature must be a value between 50 and 90" << std::endl;
        return 1;
    }

    nvmlReturn_t result;
    unsigned int device_count, i;

    result = nvmlInit();
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to initialize NVML: " << nvmlErrorString(result) << std::endl;
        return 1;
    }

    result = nvmlDeviceGetCount(&device_count);
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to query device count: " << nvmlErrorString(result) << std::endl;
        return 1;
    }

    while (true) {
        for (i = 0; i < device_count; i++) {
            nvmlDevice_t device;
            result = nvmlDeviceGetHandleByIndex(i, &device);
            if (NVML_SUCCESS != result) {
                std::cerr << "Failed to get handle for device " << i << ": " << nvmlErrorString(result) << std::endl;
                continue;
            }

            unsigned int temp;
            result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temp);
            if (NVML_SUCCESS != result) {
                std::cerr << "Failed to get temperature for device " << i << ": " << nvmlErrorString(result) << std::endl;
                continue;
            }

            // Get current fan speed
            unsigned int currentFanSpeed;
            result = nvmlDeviceGetFanSpeed_v2(device, 0, &currentFanSpeed); // Assuming fan index 0
            if (NVML_SUCCESS != result) {
                std::cerr << "Failed to get fan speed for device " << i << ": " << nvmlErrorString(result) << std::endl;
                continue;
            }

            // Get desired fan speed based on temperature
            unsigned int fanSpeed = getFanSpeed(temp, max_temp);

            // Only set the fan speed if it's different from the current speed
            if (fanSpeed != currentFanSpeed) {
                result = nvmlDeviceSetFanSpeed_v2(device, 0, fanSpeed); // Assuming fan index 0
                if (NVML_SUCCESS != result) {
                    std::cerr << "Failed to set fan speed for device " << i << ": " << nvmlErrorString(result) << std::endl;
                } else {
                   // std::cout << "Set fan speed for device " << i << " to " << fanSpeed << "%" << std::endl;
                }
            }
        }

        // Sleep for 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    nvmlShutdown();

    return 0;
}

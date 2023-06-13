#include <iostream>
#include <nvml.h>
#include <cstdlib>

void show_help() {
    std::cerr << "Usage: ./set_fan_speed <fan_speed>|auto\n";
    std::cerr << "<fan_speed>: an integer from 0-100, or 'auto' to switch to driver-controlled fan speed\n";
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        show_help();
        return 1;
    }

    nvmlReturn_t result;
    unsigned int device_count, i;

    // Initialize NVML library
    result = nvmlInit();
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to initialize NVML: " << nvmlErrorString(result) << "\n";
        return 1;
    }

    // Get device count
    result = nvmlDeviceGetCount(&device_count);
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to query device count: " << nvmlErrorString(result) << "\n";
        return 1;
    }

    // Determine whether to use automatic fan speed control
    bool useAutomaticFanSpeed = std::string(argv[1]) == "auto";

    // Loop over all devices
    for (i = 0; i < device_count; i++) {
        nvmlDevice_t device;
        result = nvmlDeviceGetHandleByIndex(i, &device);
        if (NVML_SUCCESS != result) {
            std::cerr << "Failed to get handle for device " << i << ": " << nvmlErrorString(result) << "\n";
            continue;
        }

        // Get the number of fans for this device
        unsigned int fan_count;
        result = nvmlDeviceGetNumFans(device, &fan_count);
        if (NVML_SUCCESS != result) {
            std::cerr << "Failed to query fan count for device " << i << ": " << nvmlErrorString(result) << "\n";
            continue;
        }

        // Loop over all fans
        for (unsigned int fan = 0; fan < fan_count; fan++) {
            if (useAutomaticFanSpeed) {
                // Set the fan control policy to automatic
                result = nvmlDeviceSetDefaultFanSpeed_v2(device, fan);
                if (NVML_SUCCESS != result) {
                    std::cerr << "Failed to set default fan speed for device " << i << " fan " << fan << ": " << nvmlErrorString(result) << "\n";
                    continue;
                }
                std::cout << "Set fan speed for device " << i << " fan " << fan << " to automatic\n";
            } else {
                // Set the fan speed to the specified percentage
                int fanSpeed = std::atoi(argv[1]);
                if(fanSpeed < 0 || fanSpeed > 100) {
                    std::cerr << "Fan speed must be a value between 0 and 100\n";
                    return 1;
                }
                result = nvmlDeviceSetFanSpeed_v2(device, fan, fanSpeed);
                if (NVML_SUCCESS != result) {
                    std::cerr << "Failed to set fan speed for device " << i << " fan " << fan << ": " << nvmlErrorString(result) << "\n";
                    continue;
                }
                std::cout << "Set fan speed for device " << i << " fan " << fan << " to " << fanSpeed << "%\n";
            }
        }
    }

    // Shutdown NVML library
    nvmlShutdown();

    return 0;
}

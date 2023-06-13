#include <iostream>
#include <nvml.h>
#include <cstdlib>

#define FAN_CONTROL_POLICY NVML_FAN_POLICY_MANUAL  // Change this to your desired fan control policy

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./set_fan_speed <fan_speed>" << std::endl;
        return 1;
    }
    int FAN_SPEED = std::atoi(argv[1]);  // Desired fan speed in percentage
    if(FAN_SPEED < 0 || FAN_SPEED > 100) {
        std::cerr << "Fan speed must be a value between 0 and 100" << std::endl;
        return 1;
    }
    nvmlReturn_t result;
    unsigned int device_count, i;

    // Initialize NVML library
    result = nvmlInit();
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to initialize NVML: " << nvmlErrorString(result) << std::endl;
        return 1;
    }

    // Get device count
    result = nvmlDeviceGetCount(&device_count);
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to query device count: " << nvmlErrorString(result) << std::endl;
        return 1;
    }

    // Loop over all devices
    for (i = 0; i < device_count; i++) {
        nvmlDevice_t device;
        result = nvmlDeviceGetHandleByIndex(i, &device);
        if (NVML_SUCCESS != result) {
            std::cerr << "Failed to get handle for device " << i << ": " << nvmlErrorString(result) << std::endl;
            continue;
        }

        // Get the number of fans for this device
        unsigned int fan_count;
        result = nvmlDeviceGetNumFans(device, &fan_count);
        if (NVML_SUCCESS != result) {
            std::cerr << "Failed to query fan count for device " << i << ": " << nvmlErrorString(result) << std::endl;
            continue;
        }

        // Loop over all fans
        for (unsigned int fan = 0; fan < fan_count; fan++) {
            // Set the fan control policy
            result = nvmlDeviceSetFanControlPolicy(device, fan, FAN_CONTROL_POLICY);
            if (NVML_SUCCESS != result) {
                std::cerr << "Failed to set fan control policy for device " << i << " fan " << fan << ": " << nvmlErrorString(result) << std::endl;
                continue;
            }

            // Set the fan speed
            result = nvmlDeviceSetFanSpeed_v2(device, fan, FAN_SPEED);
            if (NVML_SUCCESS != result) {
                std::cerr << "Failed to set fan speed for device " << i << " fan " << fan << ": " << nvmlErrorString(result) << std::endl;
            }
        }
    }

    // Shutdown NVML library
    nvmlShutdown();

    return 0;
}

#include <iostream>
#include <nvml.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>


void show_help() {
    std::cerr << "Usage: ./set_fan_speed [-i gpu_index] <fan_speed>|auto\n";
    std::cerr << "       -i gpu_index: Optional. The index of the GPU to set the fan speed for.\n";
    std::cerr << "       <fan_speed>: An integer from 0-100, or 'auto' to switch to driver-controlled fan speed.\n";
}

bool is_number(const std::string& s) {
    return !s.empty() && std::all_of(s.begin(), s.end(), ::isdigit);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        show_help();
        return 1;
    }

    nvmlReturn_t result;
    unsigned int device_count, i;
    int gpuIndex = -1; // Default value indicating all GPUs
    bool useAutomaticFanSpeed = false;
    int fanSpeed = -1; // Invalid default value

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
        nvmlShutdown();
        return 1;
    }

    // Parse command line arguments
    for (int argIndex = 1; argIndex < argc; ++argIndex) {
        if (strcmp(argv[argIndex], "-i") == 0) {
            if (++argIndex < argc) {
                if (is_number(argv[argIndex])) {
                    gpuIndex = std::atoi(argv[argIndex]);
                    if (gpuIndex < 0 || gpuIndex >= device_count) {
                        std::cerr << "GPU index " << gpuIndex << " is out of range. Available GPU indices are 0 to " << (device_count - 1) << ".\n";
                        nvmlShutdown();
                        return 1;
                    }
                } else {
                    std::cerr << "Invalid GPU index: " << argv[argIndex] << ". Please specify a valid number.\n";
                    nvmlShutdown();
                    return 1;
                }
            } else {
                std::cerr << "No GPU index specified after -i\n";
                show_help();
                nvmlShutdown();
                return 1;
            }
        } else if (strcmp(argv[argIndex], "auto") == 0) {
            useAutomaticFanSpeed = true;
        } else {
            fanSpeed = std::atoi(argv[argIndex]);
            if (fanSpeed < 0 || fanSpeed > 100 || !is_number(argv[argIndex])) {
                std::cerr << "Fan speed must be a value between 0 and 100\n";
                nvmlShutdown();
                return 1;
            }
        }
    }


    // Determine the start and end indices for GPU iteration
    unsigned int startIndex = (gpuIndex >= 0 && gpuIndex < device_count) ? gpuIndex : 0;
    unsigned int endIndex = (gpuIndex >= 0 && gpuIndex < device_count) ? gpuIndex + 1 : device_count;

    // Loop over specified device(s)
    for (i = startIndex; i < endIndex; i++) {
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

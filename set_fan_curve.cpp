#include <iostream>
#include <nvml.h>
#include <cstdlib>
#include <csignal>
#include <thread>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HOTSPOT_REGISTER_OFFSET 0x0002046c
#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

#define MAX_FAN_SPEED 100
#define AUTO_FAN_SPEED 0

// Forward declarations
void resetFansToAuto();
void signalHandler(int signum);
unsigned int getMaxTemperature(nvmlDevice_t device);
unsigned int readHotSpotTemperature(nvmlDevice_t device);

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


// Global variable to store device count and a flag for graceful shutdown
unsigned int device_count;
bool gracefulShutdown = false;

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

    // Register signal handler for SIGINT and SIGTERM
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    while (!gracefulShutdown) {
        for (i = 0; i < device_count; i++) {
            if (system("clear") != 0) {
                // Handle the error case or log it
                std::cerr << "Failed to clear screen." << std::endl;
            }

            nvmlDevice_t device;

            result = nvmlDeviceGetHandleByIndex(i, &device);
            if (NVML_SUCCESS != result) {
                std::cerr << "Failed to get handle for device " << i << ": " << nvmlErrorString(result) << std::endl;
                continue;
            }

            unsigned int temp;
            temp = getMaxTemperature(device);

            // Get number of fans
            unsigned int fanCount;
            result = nvmlDeviceGetNumFans(device, &fanCount);
            if (NVML_SUCCESS != result) {
                std::cerr << "Failed to get fan count for device " << i << ": " << nvmlErrorString(result) << std::endl;
                continue;
            }

           for (unsigned int fanIdx = 0; fanIdx < fanCount; fanIdx++) {
                // Get current fan speed
                unsigned int currentFanSpeed;
                result = nvmlDeviceGetFanSpeed_v2(device, fanIdx, &currentFanSpeed);
                if (NVML_SUCCESS != result) {
                    std::cerr << "Failed to get fan speed for device " << i << " fan " << fanIdx << ": " << nvmlErrorString(result) << std::endl;
                    continue;
                }

                // Get desired fan speed based on temperature
                unsigned int fanSpeed = getFanSpeed(temp, max_temp);

                // Only set the fan speed if it's different from the current speed
                if (fanSpeed != currentFanSpeed) {
                    result = nvmlDeviceSetFanSpeed_v2(device, fanIdx, fanSpeed);
                    if (NVML_SUCCESS != result) {
                        std::cerr << "Failed to set fan speed for device " << i << " fan " << fanIdx << ": " << nvmlErrorString(result) << std::endl;
                    } else {
                        std::cout << "Set fan speed for device " << i << " fan " << fanIdx << " to " << fanSpeed << "%" << std::endl;
                    }
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    resetFansToAuto();
    nvmlShutdown();

    return 0;
}

void resetFansToAuto() {
    for (unsigned int i = 0; i < device_count; i++) {
        nvmlDevice_t device;
        nvmlReturn_t result = nvmlDeviceGetHandleByIndex(i, &device);
        if (result != NVML_SUCCESS) continue;

        unsigned int fanCount;
        result = nvmlDeviceGetNumFans(device, &fanCount);
        if (result != NVML_SUCCESS) continue;

        for (unsigned int fanIdx = 0; fanIdx < fanCount; fanIdx++) {
            // Reset fan speed to automatic
            nvmlDeviceSetFanSpeed_v2(device, fanIdx, AUTO_FAN_SPEED);
        }
    }
}

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\nReset Fans to Auto\n";

    // Set flag for graceful shutdown
    gracefulShutdown = true;

    // Reset fans to automatic control
    resetFansToAuto();

    // Cleanup and close up stuff here

    // Terminate program
    exit(signum);
}

// Initialize NVML and obtain device handle before calling this function
unsigned int getMaxTemperature(nvmlDevice_t device) {
    nvmlReturn_t result;
    unsigned int gpuTemp = 0, vramTemp = 0, hotSpotTemp = 0, maxTemp;

    // Get GPU core temperature
    result = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &gpuTemp);
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to get GPU core temperature: " << nvmlErrorString(result) << std::endl;
        gpuTemp = 0; // Ignore GPU core temperature if reading fails
    }

    // TODO: Implement logic to read VRAM temperature using your method from gddr6.c
    // vramTemp = ...;

    hotSpotTemp = readHotSpotTemperature(device);

    // Determine the highest temperature
    maxTemp = std::max({gpuTemp, vramTemp, hotSpotTemp});

    std::cout << "GPU " << device << " GPU Core Temperature: " << gpuTemp << "C "
          << "VRAM Temperature: " << vramTemp << "C "
          << "Hot Spot Temperature: " << hotSpotTemp << "C" << std::endl;


    return maxTemp;
}

// Function to read Hot Spot temperature
unsigned int readHotSpotTemperature(nvmlDevice_t device) {
    unsigned int hotSpotTemp = 0;
    nvmlReturn_t result;
    nvmlPciInfo_t pciInfo; // Use nvmlPciInfo_t


    // Obtain PCI Bus ID of the device
    result = nvmlDeviceGetPciInfo(device, &pciInfo); // Pass pciInfo directly
    if (result != NVML_SUCCESS) {
        std::cerr << "Failed to get PCI Info: " << nvmlErrorString(result) << std::endl;
        return 0; // Return 0 if unable to get PCI Bus ID
    }

    // Now pciInfo contains the PCI information, including the bus ID
    std::string busId(pciInfo.busId); // Convert busId to string for use in file path

    // Correct the file path for memory-mapped I/O using pciInfo.busId
    std::string path = "/sys/bus/pci/devices/" + busId + "/resource0";

    // Open file descriptor for memory-mapped I/O
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        std::cerr << "Failed to open " << path << " for reading Hot Spot temperature." << std::endl;
        return 0;
    }

    // Memory-map the BAR0 region
    void* mapBase = mmap(nullptr, PAGE_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (mapBase == MAP_FAILED) {
        std::cerr << "Memory mapping failed." << std::endl;
        close(fd);
        return 0;
    }

    // Calculate the address and read the Hot Spot temperature
    void* regAddr = reinterpret_cast<void*>(reinterpret_cast<char*>(mapBase) + HOTSPOT_REGISTER_OFFSET);
    uint32_t regValue = *reinterpret_cast<uint32_t*>(regAddr);
    hotSpotTemp = (regValue >> 8) & 0xff; // Assuming the register format is similar

    // Cleanup
    munmap(mapBase, PAGE_SIZE);
    close(fd);

    return hotSpotTemp;
}

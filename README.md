# GPU_FAN_OC_Manager
Here is a set of tools that can be used to manage the fans on a ubunut 20.04 system with cuda 12.

How to run  set_fan_curve with a 65c target. it also updated the corntab so that it runs at boot
```
bash -c "wget https://github.com/jjziets/GPU_FAN_OC_Manager/raw/main/set_fan_curve; chmod +x set_fan_curve; cd $(pwd); nohup bash -c 'while true; do $(pwd)/set_fan_curve 65; sleep 1; done' > output.txt &; (crontab -l; echo '@reboot screen -dmS gpuManger bash -c \"while true; do $(pwd)/set_fan_curve 65; sleep 1; done\"') | crontab -"

```



# set_fan_curve.cpp

This C++ program provides a simple way to manage the fan speed of your NVIDIA GPU based on its current temperature. It uses the NVIDIA Management Library (NVML), a C-based API for monitoring and managing various states within NVIDIA (primarily Tesla) GPUs.

## How it works

The program first gets the temperature of the GPU and then calculates the appropriate fan speed using a function that determines fan speed based on the current GPU temperature and a user-defined maximum temperature.

It continuously monitors the GPU temperature and adjusts the fan speed as needed, sleeping for one second between each check.

## Usage

The program requires one argument: the maximum GPU temperature. If the current GPU temperature is within a certain range of the maximum temperature, the fan speed will be adjusted accordingly.

Here's how to run the program:

```bash
./set_fan_curve <max_temperature>
```

Where `<max_temperature>` is the desired maximum temperature for your GPU, with a valid range from 50 to 90.

## Prerequisites

You need to have the NVIDIA Management Library (NVML) installed on your system to build and run this program. Please refer to NVIDIA's official documentation for NVML installation instructions.

## Build

You can compile the program using g++ or any other compatible C++ compiler:

```bash
g++ -o set_fan_curve set_fan_curve.cpp -lnvidia-ml
```

This will generate an executable named `set_fan_curve` which you can run as mentioned above.

## Important Note

This program directly manipulates the GPU's fan speed and it can potentially harm your hardware if not used properly. Be careful to choose a safe maximum temperature for your specific GPU model. It is highly recommended to thoroughly understand what this program does and how it works before running it. Please use it at your own risk.

# set_fan_speed.cpp

This C++ program allows you to manually control the fan speed of your NVIDIA GPUs using the NVIDIA Management Library (NVML). It offers an option to set a specific fan speed, or to switch back to the automatic fan control provided by the driver.

## How it works

This program provides direct control over the GPU fan speed. It allows you to either specify a desired fan speed (as a percentage of the maximum speed) or to set the fan control back to automatic, which lets the GPU driver manage the fan speed based on the current load and temperature.

## Usage

Here's how to run the program:

```bash
./set_fan_speed <fan_speed>|auto
```

Where `<fan_speed>` is the desired fan speed as a percentage (0-100), or 'auto' to switch back to driver-controlled fan speed.

## Prerequisites

You need to have the NVIDIA Management Library (NVML) installed on your system to build and run this program. Please refer to NVIDIA's official documentation for NVML installation instructions.

## Build

You can compile the program using g++ or any other compatible C++ compiler:

```bash
g++ -o set_fan_speed set_fan_speed.cpp -lnvidia-ml
```

This will generate an executable named `set_fan_speed` which you can run as mentioned above.

## Important Note

This program directly manipulates the GPU's fan speed, which can potentially harm your hardware if not used properly. Be careful when choosing a fan speed, especially when setting a low speed for a high-performance GPU under load. It's highly recommended to understand the program's workings before running it, and use it at your own risk.


# Bash script that are based on nvidia-fan-control-linux
gpuManger4auto.sh
gpuManger8auto.sh

This file sets both the fan curve and the gpu oc as needed.
This file has be desinged to work on a system that has NVlink pairs. thefore you will have to remap the gpus index else the wrong gpus will be set. GPU0 and fan0/memoryoc0 does not match when there is NVLINKS present

This manager will oc the gpu of t-rex is running on that gpu.

## Before running insall the following 
``` 
bash -c 'sudo apt-get update; sudo apt-get -y upgrade; sudo apt-get install -y libgtk-3-0; sudo apt-get install -y xinit; sudo apt-get install -y xserver-xorg-core; sudo apt-get remove -y gnome-shell; sudo update-grub; sudo nvidia-xconfig -a --cool-bits=28 --allow-empty-initial-configuration --enable-all-gpus; wget https://raw.githubusercontent.com/jjziets/GPU_FAN_OC_Manager/main/gpuManger4auto.sh, chmod +x gpuManger4auto.sh, 
(crontab -l; echo "@reboot screen -dmS gpuManger /home/dafit/gpuManger4auto.sh pcurve") | crontab -'

```

# GPU_FAN_OC_Manager
Here is a set of tools that can be used to manage the fans on a ubunut 20.04 system with cuda 12.

This script is designed to manage the fan speed of your nvidia GPUs. It does this by running a process called `set_fan_curve` which adjusts the fan speed based on the given parameter (in this case, `65`) which is 65c target every second. The while loop is needed if the program exits due do driver errors. 


```
bash -c "wget https://github.com/jjziets/GPU_FAN_OC_Manager/raw/main/set_fan_curve; chmod +x set_fan_curve; CURRENT_PATH=\$(pwd); nohup bash -c \"while true; do \$CURRENT_PATH/set_fan_curve 65; sleep 1; done\" > output.txt & (crontab -l; echo \"@reboot screen -dmS gpuManger bash -c 'while true; do \$CURRENT_PATH/set_fan_curve 65; sleep 1; done'\") | crontab -"
```

Here's the detailed explanation of the script:

1. `wget https://github.com/jjziets/GPU_FAN_OC_Manager/raw/main/set_fan_curve;`: 
   This command fetches the `set_fan_curve` script from a GitHub repository.

2. `chmod +x set_fan_curve;`: 
   This command changes the permissions of the `set_fan_curve` script to make it executable.

3. `cd $(pwd);`: 
   This command changes the current directory to the directory from where the script is run. `$(pwd)` returns the current directory.

4. `nohup bash -c 'while true; do $(pwd)/set_fan_curve 65; sleep 1; done' > output.txt &;`: 
   This command starts an infinite loop that runs the `set_fan_curve`. If set_fan_curve exits it will restart it with a second delay. The output is redirected to `output.txt` and the process is backgrounded (`&`). The `nohup` command makes the loop continue to run even if the terminal session is closed.

5. `(crontab -l; echo '@reboot screen -dmS gpuManger bash -c \"while true; do $(pwd)/set_fan_curve 65; sleep 1; done\"') | crontab -`: 
   This command modifies the current user's crontab to include a new job that will run on system startup (`@reboot`). The job runs a `screen` session named `gpuManger` that starts the same infinite loop as above. This ensures that the fan control script will start running again after each system reboot.

This script is useful if you want to automate the management of your GPU fan speed. Remember that running the script will replace the existing fan curve, so use it carefully and make sure to test it in a controlled environment first.




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

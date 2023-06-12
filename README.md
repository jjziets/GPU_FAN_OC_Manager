# GPU_FAN_OC_Manager
Based on nvidia-fan-control-linux

This file sets both the fan curve and the gpu oc as needed.
This file has be desinged to work on a system that has NVlink pairs. thefore you will have to remap the gpus index else the wrong gpus will be set. GPU0 and fan0/memoryoc0 does not match when there is NVLINKS present

This manager will oc the gpu of t-rex is running on that gpu.

#Before running insall the following 
``` 
bash -c 'sudo apt-get update; sudo apt-get -y upgrade; sudo apt-get install -y libgtk-3-0; sudo apt-get install -y xinit; sudo apt-get install -y xserver-xorg-core; sudo apt-get remove -y gnome-shell; sudo update-grub; sudo nvidia-xconfig -a --cool-bits=28 --allow-empty-initial-configuration --enable-all-gpus; wget https://raw.githubusercontent.com/jjziets/GPU_FAN_OC_Manager/main/gpuManger4auto.sh, chmod +x gpuManger4auto.sh, 
(crontab -l; echo "@reboot screen -dmS gpuManger /home/dafit/gpuManger4auto.sh pcurve") | crontab -'

```

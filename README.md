# GPU_FAN_OC_Manager
Based on nvidia-fan-control-linux

This file sets both the fan curve and the gpu oc as needed.
This file has be desinged to work on a system that has NVlink pairs. thefore you will have to remap the gpus index else the wrong gpus will be set. GPU0 and fan0/memoryoc0 does not match when there is NVLINKS present

This manager will oc the gpu of t-rex is running on that gpu.


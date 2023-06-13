#!/bin/bash
# vim: set foldenable foldmethod=marker ts=4 sw=4:
# License Info                                                           {{{1
# Fan Control Script w/ Fan Curve
# Copyright (C) 2019  Barry Van Deerlin#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

#####   Configurable Settings   #####                                    {{{1
#OC settings
MemoryOC=2700 #memory c
CoreClockLimit=1250 #core limits
GPU_OC=0


# FanControl Configuration Path                                          {{{1
fanConfig="$(getent passwd $(id -un) | cut -d ':' -f6)/.fancontrol"

# Default Fan Speed Setting                                              {{{1
defaultSpeed=60

# Persistent Fan Curve Refresh Interval                                  {{{1
refresh=10

# Fan Curve Settings                                                     {{{1
# Day Curve Start Time (24 Hour Time)
dCurveStart=12
# Night Curve Start Time (24 Hour Time)
nCurveStart=23
# Night Curve Enable/Disable (true/false)
# Disable if you don't want seperate day and night curves
nCurveEnabled=false

# Fan Curve Temperature Thresholds (In Celsius)                          {{{2
# The temperature threshold is the temperature at which the the script
# applies a different fan curve point. When Adding Temperature Thresholds
# you must also add curve points.
# There must always be one less temperature threshold (Excluding MAXTHRESHOLD)
# then there is curve points for the script to work.

MAXTHRESHOLD=65    # Fans will run at 100% if hotter than this temperature
tempThresh[0]=60   # <-- Apply curve[0] if hotter than
tempThresh[1]=55   # <-- Apply curve[1] if hotter than
tempThresh[2]=50   # <-- Apply curve[2] if hotter than
tempThresh[3]=40   # <-- Apply curve[3] if hotter than
tempThresh[4]=30   # <-- Apply curve[4] if hotter than
                   # """ Apply curve[5] if cooler than

# Fan Curve Points                                                       {{{2
# The curve point is the Fan Speed Percentage applied at a given temperature
# threshold. When Adding Curve Points you must also add temperature thresholds.
# There must always be one less temperature threshold (Excluding MAXTHRESHOLD)
# then there is curve points for the script to work properly.

# Day Curve   Night Curve
dCurve[0]=90  nCurve[0]=90
dCurve[1]=80  nCurve[1]=80
dCurve[2]=70  nCurve[2]=70
dCurve[3]=50  nCurve[3]=50
dCurve[4]=40  nCurve[4]=40
dCurve[5]=30  nCurve[5]=30

##### End Configurable Settings #####                                    {{{1
# Export Display (For Headless Use)                                      {{{1
echo "Start a new X server"

if pgrep -x "Xorg"  > /dev/null
        then
	pkill Xorg
	export DISPLAY=':0'
	else
#	X :0 &
	export DISPLAY=':0'
fi

# Paths to the utilities we will need
SMI='/usr/bin/nvidia-smi'
SET='/usr/bin/nvidia-settings'

# Get Number of Connected GPUs                                           {{{1
numGPUs=$(nvidia-smi --query-gpu=count --format=csv,noheader -i 0)

# Function to enable manual fan control state on first run               {{{1
initFCS()
{
    FanControlStates=($(nvidia-settings -q GPUFanControlState | grep 'Attribute' | awk -vFS=': ' -vRS='.' '{print $2}'))

    for i in ${FanControlStates[@]}; do
        if [ $i -eq 0 ]; then
            xinit   ${SET} -a "GPUFanControlState=1"  -a "[gpu:$i]/GPUFanControlState=1"--  :0 -once > /dev/null 2>&1
            echo "Fan Control State Enabled"
            break
        fi
    done

    #close Xserve
    #pkill -15 Xorg
}
# function for setting gpus OC
GpuOC_profile()
{
    #test if t-rex is running before starting the search
    if pgrep -x "t-rex"  > /dev/null
        then
	if [[ $GPU_OC -eq 0 ]]; then
		let GPU_OC=1 
		xinit  ${SET}   -a [gpu:0]/GPUFanControlState=1 -a [gpu:0]/GPUPowerMizerMode=1 -a [gpu:0]/GPUMemoryTransferRateOffsetAllPerformanceLevels=$MemoryOC -a [fan:0]/GPUTargetFanSpeed=100 \
				-a [gpu:1]/GPUFanControlState=1 -a [gpu:1]/GPUPowerMizerMode=1 -a [gpu:1]/GPUMemoryTransferRateOffsetAllPerformanceLevels=$MemoryOC -a [fan:1]/GPUTargetFanSpeed=100 \
                                -a [gpu:2]/GPUFanControlState=1 -a [gpu:2]/GPUPowerMizerMode=1 -a [gpu:2]/GPUMemoryTransferRateOffsetAllPerformanceLevels=$MemoryOC -a [fan:2]/GPUTargetFanSpeed=100 \
                                -a [gpu:3]/GPUFanControlState=1 -a [gpu:3]/GPUPowerMizerMode=1 -a [gpu:3]/GPUMemoryTransferRateOffsetAllPerformanceLevels=$MemoryOC -a [fan:3]/GPUTargetFanSpeed=100 --  :0 -once
		nvidia-smi --lock-gpu-clocks=$CoreClockLimit
	fi
     else
        if [[ $GPU_OC -eq 1 ]]; then
                let GPU_OC=0
                xinit  ${SET}   -a [gpu:0]/GPUPowerMizerMode=1 -a [gpu:0]/GPUMemoryTransferRateOffsetAllPerformanceLevels=0 \
                                -a [gpu:1]/GPUPowerMizerMode=1 -a [gpu:1]/GPUMemoryTransferRateOffsetAllPerformanceLevels=0 \
                                -a [gpu:2]/GPUPowerMizerMode=1 -a [gpu:2]/GPUMemoryTransferRateOffsetAllPerformanceLevels=0 \
                                -a [gpu:3]/GPUPowerMizerMode=1 -a [gpu:3]/GPUMemoryTransferRateOffsetAllPerformanceLevels=0 --  :0 -once
                nvidia-smi  -rgc
        fi
     fi
}


# Function that applies Fan Curve                                        {{{1
runCurve()
{

     GpuOC_profile
     if pgrep -x "t-rex"  > /dev/null # i trex is running then dont adjust fans speeds
     then
	return
     fi 

    # Get GPU Temperature and Current FanSpeed	

    IFS=$'\n'
    gputemp=($(nvidia-smi --query-gpu=temperature.gpu --format=csv,noheader))
    currentSpeed=($(nvidia-smi --query-gpu=fan.speed --format=csv,noheader | awk '{print $1}'))
    unset IFS


    [ $nCurveEnabled ] && cTime=$(date +'%H') || cTime=$dCurveStart # Get the time

    # Checks time to apply day or night curve
    [ $cTime -lt $dCurveStart -o $cTime -ge $nCurveStart ] && curve=("${nCurve[@]}") || curve=("${dCurve[@]}")

    # Loop through each GPU
    for i in $(seq 0 $((numGPUs-1))); do
        speed=100
	Fspeed[$i]=$speed
	echo  "Temp GPU$i:${gputemp[$i]} FanSpeed:${currentSpeed[$i]} "
        # Set speed to appropriate value from curve
        if [ ${gputemp[$i]} -lt $MAXTHRESHOLD ]; then
            cPoint=$((${#curve[@]}-1))
            for c in $(seq 0 $cPoint); do
                index=$c
#		echo "c = $c"
                case "$c" in
                    $cPoint)
                        comparison=-lt
                        index=$((c-1))
                        ;;
                    *)
                        comparison=-ge
                        ;;
                esac
                [ ${gputemp[$i]} $comparison ${tempThresh[$index]} ] && { speed=${curve[$c]}; break; }
            done
        fi

        # Apply fan speed if speed has changed
	Fspeed[$i]=$speed
	echo "fan speed $i $speed"
#        [ $speed -lt $((currentSpeed[i]-1)) -o $speed -gt $((currentSpeed[i]+1)) ] && Fspeed[$i]=$speed

    done

        if [[ "${currentSpeed[2]}" != "${Fspeed[2]}" ]]; then
			xinit ${SET}    -a [gpu:0]/GPUFanControlState=1    -a [fan:0]/GPUTargetFanSpeed=${Fspeed[2]} --  :0 -once
			echo "GPU0 current ${currentSpeed[2]} and new speed ${Fspeed[2]}"
	fi
	if [[ "${currentSpeed[3]}" != "${Fspeed[3]}" ]]; then
			xinit ${SET}      -a [gpu:1]/GPUFanControlState=1  -a [fan:1]/GPUTargetFanSpeed=${Fspeed[3]} --  :0 -once
               		echo "GPU1 current ${currentSpeed[3]} and new speed ${Fspeed[3]}"
	fi
	if [[ "${currentSpeed[0]}" != "${Fspeed[0]}" ]]; then
			xinit ${SET}    -a [gpu:2]/GPUFanControlState=1  -a [fan:2]/GPUTargetFanSpeed=${Fspeed[0]} --  :0 -once
                	echo "GPU2 current ${currentSpeed[0]} and new speed ${Fspeed[0]}"
	fi
	if [[ "${currentSpeed[1]}" != "${Fspeed[1]}" ]]; then
			xinit ${SET}     -a [gpu:3]/GPUFanControlState=1  -a [fan:3]/GPUTargetFanSpeed=${Fspeed[1]} --  :0 -once
               		echo "GPU3 current ${currentSpeed[1]} and new speed ${Fspeed[1]}"
	fi



}

# Function that gets GPU Fan Stats and displays them                     {{{1
getInfo()
{
    IFS=$'\n'
    # Retrieve GPU Names,  Fan Speed, and Temperature
    query=($(nvidia-smi --query-gpu=name,fan.speed,temperature.gpu --format=csv,noheader))
    # Retrieve GPU Fan RPM
    query_rpm=($(nvidia-settings -q GPUCurrentFanSpeedRPM | grep "fan:" | awk -F ': ' -vRS='.' '{print $2}'))

    # Summary format
    # Nvidia Fan Info
    # | Card |              | Fan Speed |   | Fan RPM | | GPU Temp |
    # Geforce GTX 1080 Ti        50%            1600         53°

    # Print out Header
    printf "Nvidia Fan Info\n| Card |\t\t| Fan Speed |\t| Fan RPM |\t| GPU Temp |\n"

    # Loop through GPUs to compile summary
    for i in $(seq 0 $((numGPUs-1))); do
        card=$(awk -F ', ' '{print $1}' <<< ${query[$i]})
        fan_speed=$(awk -F ', ' '{print $2}' <<< ${query[$i]} | awk '{print $1}')
        fan_rpm=${query_rpm[$i]}
        temp=$(awk -F ', ' '{print $3}' <<< ${query[$i]})
        printf "%s: %s\t     %s%%\t    %s\t     %s°\n" $i $card $fan_speed $fan_rpm $temp
    done

    unset IFS
}

# Parse and Execute Arguments passed to script                           {{{1
case "$1" in
    # Set Fan Speed for all GPU Fans                                     {{{2
    set|s)
        case "$2" in
            # Enable Fan Curve (Use with Cron)
            curve|c)        speed="curve" ;;

            # Set Speed to Default
            default|d)      speed=$defaultSpeed ;;

            # Set Speed to Max
            max|m|100)      speed=100 ;;

            # Turn Fans Off
            off)            speed=0 ;;

            # Set Fan Speed Manually
            [0-9]|[1-9][0-9])   speed=$2 ;;

            # Improper Input Given
            *)          echo "Usage: $0 $1 {# Between 0 - 100|d (default)|m (max)|off|curve}"; exit 2 ;;
        esac

        case "$speed" in
            curve)
                echo curve > $fanConfig # Change Configuration to Curve
                $0 curve # Run Fan Curve
                ;;
            *)
                echo manual > $fanConfig # Enabling Manual Control and Disabling Fan Curve
                initFCS
                 xinit ${SET}  -a "GPUTargetFanSpeed=$speed"   --  :0 -once
                ;;
        esac
        ;;

    # For testing Individual GPU Fan Settings                            {{{2
    dx)
        # Test if Proper Input was given
        # Is input $2 a valid GPU index? Is input $3 a number that is less than or equal to 100?
        re='^[0-9]{,2}$'
        [ $# -eq 3 ] && [[ $2 =~ $re && $2 -lt $numGPUs ]] && [[ $3 =~ $re || $3 -eq 100 ]] \
        &&  xinit ${SET} \
            -a "[gpu:$2]/GPUFanControlState=1" \
            -a "[fan:$2]/GPUTargetFanSpeed=$3"   --  :0 -once 

        [ $? -ne 0 ] && { echo "Usage: $0 $1  gpuIndex  FanSpeed Between 0 - 100"; exit 2; }
        ;;

    # Applies Fan Curve (For use with cron)                              {{{2
    curve|c)
        # Checks if Configuration File exists and create it if it doesn't
        [ ! -f $fanConfig ] && echo "curve" > $fanConfig

        # Run fan curve if configuration is set to curve
        case "$(cat $fanConfig)" in
            curve)
                initFCS
                runCurve
	;;
        esac
        ;;

    # Applies Persistant Fan Curve (For use without cron)                {{{2
    pcurve|pc)
        echo "pcurve" > $fanConfig
        initFCS
        # Run while configuration is set to pcurve
        while [ "$(cat $fanConfig)" == "pcurve" ]; do
            runCurve
            sleep $refresh
        done
        ;;

    # Display GPU Fan and Temp Status                                    {{{2
    info|i)
        getInfo
        ;;

    # Display Version Info                                               {{{2
    --version|-v)
        echo "$0 v0.1 Copyright (C) 2019 Barry Van Deerlin"
        ;;

    # Incorrect Usage                                                    {{{2
    *)
        echo "Usage: $0 {set|dx(diagnose)|curve|pcurve|info)}"
        exit 2
esac

# Exit                                                                   {{{1
exit 0

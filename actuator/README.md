# Cuddlebot Firmware

This actuator firmware code is an update/adaptation of [mikepb's version of the UBC cuddlebot firmware](https://github.com/mikepb/cuddlebot-firmware/tree/master/actuator).

Major changes include:
- Implementation of smoothing command ([comm.c](comm.c))
- Bi-directional calibration ([motor.c](motor.c))
- Discrete calibration at timed intervals ([main.c](main.c))

## STM32 Setup/Installation

The following descriptions are copied off of [this installation guide](http://fab.cba.mit.edu/classes/4.140/tutorials/EmbeddedProgramming/stm32.html) (in case the website is no longer available in the future). This setup is accurate as of August 2015 for Ubuntu 14.04. I think the updated version of the description can be found here[guide](http://fab.cba.mit.edu/classes/863.13/tutorials/EmbeddedProgramming/stm32.html) as the date of 20/06/2016
 

```bash
# Update all sources and upgrade system
sudo apt-get -y update && sudo apt-get -y upgrade && sudo apt-get -y autoremove
# Install toolchain prerequisites (Autoreconf & LibUSB & Git)
sudo apt-get install dh-autoreconf libusb-1.0.0-dev git
# Add PPA to system
sudo add-apt-repository ppa:terry.guo/gcc-arm-embedded
# Update sources
sudo apt-get update
# Install toolchain package
sudo apt-get install gcc-arm-none-eabi
# Install STLink
# In local folder:
git clone https://github.com/texane/stlink.git
cd stlink
./autogen.sh
./configure
make
# Verify STLink tools compilation
ls -la | grep st-
# Install STLINK tools
make install
# Verify access
cd .. && st-util
```

## Building

```bash
make
```

## Flashing

```bash
make flash
```
Note that when flashing, the cord should be plugged into the `USB ST-LINK` port and not the `USB USER` port.


## Debugging

https://github.com/texane/stlink

Run the debug server:

```bash
make debug-serve
```

Connect to the debug server:

```bash
$ make debug
...
(gdb) tar extended-remote :4242
...
(gdb) load
Loading section .text, size 0x458 lma 0x8000000
Loading section .data, size 0x8 lma 0x8000458
Start address 0x80001c1, load size 1120
Transfer rate: 1 KB/sec, 560 bytes/write.
(gdb)
...
(gdb) continue
```

## Accessing additional sensors

The firmware does not make use of sensor data other than for
calculating the position of the motor. The unused sensors and reasons
for not using the data follow:

- Internal temperature: slow to sample and the damage is already done
  once we sense a dangerous temperature.
- KMZ60 temperature: accuracy is very limited compared to internal 
  temperature sensor and the damage is already done once we sense a
  dangerous temperature.
- Torque: change in value on high torque is small and the robot is 
  designed to be pliable, making the torque sensor less useful.
- Current: the current is related to the voltage output, which is
  related to the PWM output, meaning we have access to the same 
  information via the PWM setting. Furthermore, we may simply limit
  the PWM output to prevent overheating the motor altogether.

You may find code to access the sensors in commit 88c597f.

# Cuddlebot Firmware

See internal docs for architecture details.


## Building

```bash
make
```

## Flashing

```bash
make flash
```


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

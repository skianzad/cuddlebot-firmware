# Cuddlebot Firmware

This repository contains various firmware projects for the Cuddlebot.

Before getting started, please also checkout the ChibiOS submodule:

```sh
$ git submodule init
$ git submodule update
```

You will find the actuator firmware under `actuator/`. The
`pressure-sensor/` directory contains an implementation for the pressure
sensor using an STM32F3.

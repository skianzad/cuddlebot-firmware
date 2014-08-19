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

# SpEC Framework

This is a basic Linux framework for ANL's SpEC I board. It contains various modules that can be used to build an ARM application compiled using GCC 4.4 toolchain.

### Supported features:
- Config file parser
- One-shot timer
- CAN Sockets
- UDP Sockets
- TCP/IP Sockets (IPv4 & IPv6)
- Basic Net Utilities (get IP address, address family)
- Threading (lock/unlock mutex)
- PWM (set frequency, duty cycle)
- Serial I/O

### Upcoming features:
- Pilot Server (read and set pilot pin state)
- Prox Server (read and set proximity pin state)
- PWM - get duty cycle
- GPIO manager (set pin states)
- TLS 1.2/1.3 support
- EXI codec
- SLAC
- Logger
- MQTT

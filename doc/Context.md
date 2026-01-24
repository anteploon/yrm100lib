# Context (yrm100_context_t)

See **yrm100_context_t** in **yrm100_types.h**

In theory one can have multiple YRM100 devices connected to the system and **yrm100_context_t** is sort of a handle to that device.

Holds a serial port handle to the device, some state flags and buffers.

| Field                                                                   | Usage                                                                              |
| ----------------------------------------------------------------------- | ---------------------------------------------------------------------------------- |
| char \***serial_port_name**                                             | Just a string pointing to the serial device (/dev/ttyUSB0 or COM7 for example)     |
| serial_port_t **serial_port**                                           | Actual handle to the serial device                                                 |
| int **last_error_code**                                                 | Last error code - call **yrm100_error_code_to_string()** to convert it to string   |
| bool **is_initialized**                                                 | Set to true if the serial port is successfully opened and initialized              |
| bool **is_multi_poll_running**                                          | Set to true if multi_poll process is running                                       |
| yrm100_rfid_tag_t \***multi_poll_target**                               | Multi poll target buffer                                                           |
| unsigned short **multi_poll_maximum_tag_count**                         | Multi poll tag limit                                                               |
| unsigned short **multi_poll_tag_count**                                 | Multi poll cursor / count of reads                                                 |

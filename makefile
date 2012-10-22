MCU = atmega8
MCU_ = m8

FLASHSIZE = 0x2000

TARGET = main
ASRC =
CSRC = $(TARGET).c 44780.c owi_uart.c

include	mk

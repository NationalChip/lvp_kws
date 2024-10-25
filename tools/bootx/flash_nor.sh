#!/bin/bash

BOOTXCMD="download 0x0 ../../output/mcu_nor.bin;"
BOOTXCMD="$BOOTXCMD reboot;"

if [ $# == 1 ]; then
sudo ./bootx -m grus -c $BOOTXCMD -d /dev/ttyUSB$1
else
sudo ./bootx -m grus -c $BOOTXCMD -d /dev/ttyUSB$1 $2 $3
fi



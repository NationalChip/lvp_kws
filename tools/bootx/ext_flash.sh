#!/bin/bash

if [ $# == 1 ]; then
    sudo ./bootx -m grus -b grus_generic_spi_nor_flash.boot -c "download 0 ../../output/ext_flash.bin;reboot" -d /dev/ttyUSB$1
    sudo ./bootx -m grus -c "download 0 ../../output/mcu_flash.bin;reboot" -d /dev/ttyUSB$1 $2 $3
else
    sudo ./bootx -m grus -b grus_generic_spi_nor_flash.boot -c "download 0 ../../output/ext_flash.bin;reboot" -d /dev/ttyUSB$1
    sudo ./bootx -m grus -c "download 0 ../../output/mcu_flash.bin;reboot" -d /dev/ttyUSB$1 $2 $3
fi
cd ..


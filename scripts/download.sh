#!/bin/bash

# partition start address
MCU_OFFSET=0x0

# bin file path
MCU_PATH="mcu_nor.bin"

# download command
MCU_CMD="download ${MCU_OFFSET} ${MCU_PATH}"
ALL_CMD="${MCU_CMD}; reboot"


#!/bin/bash

if [ $# -ne 1 ]; then
 echo "Usage: $0 [gw_id]"
 exit -1
fi

ESP_BAUD=921600
GW_ID=$1
NV_BIN="../lcg100-nv-${GW_ID}.bin"

pushd build/
esptool.py --chip esp32 --port ${ESP_PORT:=/dev/ttyUSB0} --baud ${ESP_BAUD:-115200} erase_flash
esptool.py --chip esp32 --port ${ESP_PORT:=/dev/ttyUSB0} --baud ${ESP_BAUD:-115200} write_flash --flash_mode dio --flash_freq 40m --flash_size 4MB 0x8000 partition_table/partition-table.bin 0xd000 ota_data_initial.bin 0x1000 bootloader/bootloader.bin 0x10000 new_env.bin 0x9000 $NV_BIN
popd

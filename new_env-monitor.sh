#!/bin/bash

# if [ $# -ne 1 ]; then
#  echo "Usage: $0"
#  exit -1
# fi

pushd build/
idf_monitor.py -p ${ESP_PORT:=/dev/ttyUSB0} -b ${ESP_BAUD:-115200} new_env.elf
popd

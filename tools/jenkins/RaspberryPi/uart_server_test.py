#!/usr/bin/python
# -*- coding: UTF-8 -*-
from bluepy import btle
from bluepy.btle import Scanner,DefaultDelegate
import sys


#configure const value
UART_SERVICE_UUID="6e400001-b5a3-f393-e0a9-e50e24dcca9e"
DEVICE_MAC="00:ff:00:ff:00:ff"
#exit code
TEST_SUCCESS=0
EXIT_CODE_ADV_NOT_FOUND=1
EXIT_CODE_SERVICES_NOT_FOUND=2
#test result
found_device=0
found_services=0



#handle exit errorcode
def exit_program(err_no,description):
    print(description)
    sys.exit(err_no)


#scan
print("scanning...")
devices = Scanner().scan(5.0)
#find device
for dev in devices:
    if dev.addr == DEVICE_MAC:
        print("device found!")
        found_device=1
        break
#handle error
if found_device == 0 :
    exit_program(EXIT_CODE_ADV_NOT_FOUND,"EXIT_CODE_ADV_NOT_FOUND")


#connect
print("connecting...")
dev=btle.Peripheral(DEVICE_MAC)
print("connected!")
#find services
for ser in dev.getServices():
    if str(ser.uuid)==UART_SERVICE_UUID:
        print("services found!")
        found_services=1
        break
dev.disconnect()


#handle error
if found_services == 0 :
    exit_program(EXIT_CODE_SERVICES_NOT_FOUND,"EXIT_CODE_SERVICES_NOT_FOUND")
else:
    exit_program(TEST_SUCCESS,"TEST_SUCCESS")


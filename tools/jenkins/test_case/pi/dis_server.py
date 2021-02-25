#!/usr/bin/python
# -*- coding: UTF-8 -*-
from bluepy import btle
from bluepy.btle import Scanner,DefaultDelegate
import sys

#############configure const value####################
#the first element is serveces uuid , the behand is chara uuid
#the services readback maybe are random order.
database_standard=[
    "00001800-0000-1000-8000-00805f9b34fb",#services uuid
    "00002a00-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a01-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a04-0000-1000-8000-00805f9b34fb",#chara uuid

    "00001801-0000-1000-8000-00805f9b34fb",#services uuid
    "00002a05-0000-1000-8000-00805f9b34fb",#chara uuid

    "0000180a-0000-1000-8000-00805f9b34fb",#services uuid
    "00002a29-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a24-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a25-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a27-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a26-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a28-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a23-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a2a-0000-1000-8000-00805f9b34fb",#chara uuid
    "00002a50-0000-1000-8000-00805f9b34fb",#chara uuid

    "00002600-0000-1000-8000-00805f9b34fb",#services uuid
    "00007000-0000-1000-8000-00805f9b34fb",#chara uuid
    "00007001-0000-1000-8000-00805f9b34fb",#chara uuid
]
DEVICE_MAC="00:ff:00:ff:00:ff"
#DEVICE_MAC="11:22:33:33:22:11"
#exit code
TEST_SUCCESS=0
EXIT_CODE_ADV_NOT_FOUND=1
EXIT_CODE_DATABASE_ERROR=2

#test result
found_device=0
database_error=0
database_read=[]


#handle exit errorcode
def exit_program(err_no,description):
    print(description)
    sys.exit(err_no)

#find if the item in the list
def find_if_item_in_list(item,list_in,list_length):
    found = 0
    for i in range(list_length):
        if item == list_in[i]:
            found = 1
            break
    return found

#scan
print("scanning...")
devices = Scanner().scan(5.0)

#find device
for dev in devices:
    #print(dev.addr)
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

#read all services and char
print("read services...")
for ser in dev.getServices():
    database_read.append(str(ser.uuid))
    for chara in ser.getCharacteristics():
        database_read.append(str(chara.uuid))

#database compare
print("compare services...")
if len(database_standard) == len(database_read):
    for i in range(len(database_standard)):
        #make sure every item in the read list.
        if find_if_item_in_list(database_standard[i] , database_read , len(database_read)) == 0:
            database_error=1
            break
else:
    database_error=1
if database_error == 1:
    print(database_read)
    exit_program(EXIT_CODE_DATABASE_ERROR,"EXIT_CODE_DATABASE_ERROR")

#test ok
dev.disconnect()
exit_program(TEST_SUCCESS,"TEST_SUCCESS")


import sys
import RPi.GPIO as GPIO
import time

SWD_SEL0     = 5
SWD_SEL1     = 22
SWD_SEL2     = 27
SWD_SEL3     = 17

# init GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(SWD_SEL0,GPIO.OUT)
GPIO.setup(SWD_SEL1,GPIO.OUT)
GPIO.setup(SWD_SEL2,GPIO.OUT)
GPIO.setup(SWD_SEL3,GPIO.OUT)

sel = int(sys.argv[1]) 
if sel&0x1 == 0:
    GPIO.output(SWD_SEL0,GPIO.LOW)
else:
    GPIO.output(SWD_SEL0,GPIO.HIGH)
if sel&0x2 == 0:
    GPIO.output(SWD_SEL1,GPIO.LOW)
else:
    GPIO.output(SWD_SEL1,GPIO.HIGH)
if sel&0x4 == 0:
    GPIO.output(SWD_SEL2,GPIO.LOW)
else:
    GPIO.output(SWD_SEL2,GPIO.HIGH)
if sel&0x8 == 0:
    GPIO.output(SWD_SEL3,GPIO.LOW)
else:
    GPIO.output(SWD_SEL3,GPIO.HIGH)
    



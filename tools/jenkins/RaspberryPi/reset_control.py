import sys
import RPi.GPIO as GPIO
import time

# define Raspberry Pi PIN
BX_P16_PIN   = 26
BX_RST_PIN   = 6

# init GPIO
GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(BX_P16_PIN,GPIO.OUT)
GPIO.setup(BX_RST_PIN,GPIO.OUT)

# set P16 pin
if sys.argv[1]=="BOOT_FROM_FLASH":
    GPIO.output(BX_P16_PIN,GPIO.LOW)
    print("set P16 low ok!")
if sys.argv[1]=="BOOT_FROM_UART":
    GPIO.output(BX_P16_PIN,GPIO.HIGH)
    print("set P16 high ok!")

# set reset pin
time.sleep(0.1)
GPIO.output(BX_RST_PIN,GPIO.HIGH)
time.sleep(0.1)
GPIO.output(BX_RST_PIN,GPIO.LOW)
print("set reset pin ok!")


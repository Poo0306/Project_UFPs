import machine
import esp32
import network
import socket
import os
import sdcard
import utime
import ussl
from machine import UART
from machine import RTC, Pin, SPI, SoftSPI, SoftI2C
import sds011
import ntptime
import ujson
import dht
import urequests as requests
import ufirebase as firebase
import ufirestore as firestore
import esp
from firebase_auth.firebase_auth import AuthSession
from firebase_auth import FirebaseAuth
from ufirestore.json import FirebaseJson
# from requests import session

sd = sdcard.SDCard(machine.Pin(13), machine.Pin(12))
os.mount(sd, '/sd')
print('SD Card mounted')

# write text to a file
f = open('/sd/test.txt', 'w')
f.write('Hello SD Card\n')
f.close()

# # config
# WIFIssid = "ENGIOT"
# WIFIpsw = "coeai123"

# room = 'C4_lab'
# device_ID1 = 'Test'

# ## White board swaps between indoor and outdoor
# sensor = dht.DHT22(machine.Pin(27))

# ## Choose dust sensor
# uart = UART(1, baudrate=9600, tx=18, rx=4)
# dust_sensor = sds011.SDS011(uart)
# print('sleeping for 30 seconds...')
# dust_sensor.wake()
# utime.sleep(10)

# count = 0

# led_pin = machine.Pin(2, Pin.OUT) #built-in LED pin
# led_pin.value(1)

# inTemps, inHums, pm10s, pm25s = [], [], [], []

# now = ""

# def connect():
#     wlan = network.WLAN(network.STA_IF)
#     if not wlan.isconnected():
#         print('connecting to network...', WIFIssid)
#         wlan.active(True)
#         wlan.connect(WIFIssid, WIFIpsw)
#         count = 0
#         while not wlan.isconnected():
#              if(count > 60):
#                print('Cannot connect to network...')
#                utime.sleep(180)
#                machine.reset()
#              else: 
#                print('.', end = '')
#                utime.sleep(1)
#                count = count + 1
#                pass
#     print()
#     print('network config: {}'.format(wlan.ifconfig()))

# def set_time():
#     ntptime.settime()
#     tm = utime.localtime()
#     tm = tm[0:3] + (0,) + tm[3:6] + (0,)
#     machine.RTC().datetime(tm)
#     print('current time: {}'.format(utime.localtime()))

# def mean(arr):
#     sum = 0.00
#     for x in arr:
#         sum = sum + x
#     avg = sum / len(arr)
#     return avg

# def indoorDust(now, minute):
    
#     global inTemps, inHums, pm10s, pm25s

#     it = 0
#     ih = 0
#     pm10 = 0
#     pm25 = 0

#     ### Indoor Environment Senser ###
#     try:
#         sensor.measure()
#         it = sensor.temperature()
#         ih = sensor.humidity()
#         inTemps.append(it)
#         inHums.append(ih)
#     except:
#         inTemps.append(1.00)
#         inHums.append(1.00)
    
#      # For Dust
#     try:
#         dust_sensor.read()
#         dust_sensor.packet_status
#         pm25 = dust_sensor.pm25
#         pm10 = dust_sensor.pm10
#         pm25s.append(pm25)
#         pm10s.append(pm10)
#     except:
#         pm25s.append(0.00)
#         pm10s.append(0.00)

#     print(now, it, ih, pm10, pm25)
#     utime.sleep(2)

#     if(minute % 1 == 0):
#         inTemp = mean(inTemps)
#         inHum = mean(inHums)
#         pm10 = mean(pm10s)
#         pm25 = mean(pm25s)

#         print()
#         line = now + ", " + str(inTemp) + ", "+str(inHum)  + ", "+ str(pm10) + ", " + str(pm25) +  "\n"
#         print(line)

#         message1 = {
#             "Timestamp": now,
#             "IndoorTemperature": inTemp,
#             "IndoorHumidity": inHum,
#             "PM 10": pm10,
#             "PM 25": pm25,
#         }

#         utime.sleep(1)

#         #try:
#         led_pin.value(1)

#         # Publish to Firebase Realtime Database
#         doc = FirebaseJson()
#         # path1 = "UFPs/" + room + "/" + device_ID1 + "/" + now +  "/"
#         doc.set = ("IndoorTemperature/floatValue", inTemp)
#         doc.set = ("IndoorHumidity/floatValue", inHum)
#         doc.set = ("PM 10/floatValue", pm10)
#         doc.set = ("PM 25/floatValue", pm25)
#         doc.set = ("Timestamp/stringValue", now)
#         response = firestore.create("UFPs", doc, document_id=None, bg=True, cb=None)
#         print(response)
#         # firestore.create(PATH= "UFPs",DOC = doc, bg=0, cb=None)
#         # firebase.put(path1, message1, bg=0, id=0) # try bg = false

#         # print(path1)
#         # print(message1)

#         inTemps = []
#         inHums = []
#         pm10s = []
#         pm25s = []

#         led_pin.value(0)

#         utime.sleep(5)

# def main():

#     connect()

#     toggle = 0

#     #Initialize the onboard LED as output
#     led = machine.Pin(2,machine.Pin.OUT)

#     # Toggle LED functionality
#     def BlinkLED(timer_one):
#         global toggle
#         if toggle == 1:
#             led.value(0)
#             toggle = 0
#         else:
#             led.value(1)
#             toggle = 1
    
#     # Initialize the SD card
#     spi=SoftSPI(1,sck=Pin(13),mosi=Pin(14),miso=Pin(15))
#     sd=sdcard.SDCard(spi,Pin(12))

#     # Create a instance of MicroPython Unix-like Virtual File System (VFS),
#     vfs=os.VfsFat(sd)

#     # Mount the SD card
#     os.mount(sd,'/sd')

#     # Debug print SD card directory and files
#     print(os.listdir('/sd'))

#     # Create / Open a file in write mode.
#     # Write mode creates a new file.
#     # If  already file exists. Then, it overwrites the file.
#     file = open("/sd/ufps.txt","w")

#     # Write sample text
#     for i in range(5):
#         file.write("Sample text = %s\r\n" % i)
        
#     # Close the file
#     file.close()

#     # Again, open the file in "append mode" for appending a line
#     file = open("/sd/ufps.txt","a")
#     file.write("Appended Sample Text at the END \n")
#     file.close()

#     # Open the file in "read mode". 
#     # Read the file and print the text on debug port.
#     file = open("/sd/ufps.txt", "r")
#     if file != 0:
#         print("Reading from SD card")
#         read_data = file.read()
#         print (read_data)
#     file.close()

#     # Initialize timer_one. Used for toggling the on board LED
#     timer_one = machine.Timer(0)

#     # Timer one initialization for on board blinking LED at 200mS interval
#     timer_one.init(freq=5, mode=machine.Timer.PERIODIC, callback=BlinkLED)

#     # api_key = "AIzaSyABhPGdjCC4XEDK2x-Rfx-QsSX9aY59s-o"
#     # auth = FirebaseAuth(api_key)

#     # email = "poo.t2546@gmail.com"
#     # password = "yokpuwadech03"
#     # auth.sign_in(email, password)

#     # session = auth.session
#     # access_token = session.access_token

#     # print("auth: ", access_token)
    
#     # firestore.set_project_id("ultrafine-particles")
#     # firestore.set_access_token(access_token)
    
#     #Need to be connected to the internet before setting the local RTC.
#     set_time()
#     rtc = machine.RTC()
#     utc_shift = 7
#     (year, month, mday, week_of_year, hour, minute, second, milisecond) = rtc.datetime()
#     rtc.init((year, month, mday, week_of_year, hour + utc_shift, minute, second, milisecond))

#     while(True):

#         t = rtc.datetime()
#         now = '{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}'.format(t[0], t[1], t[2], t[4], t[5], t[6])            
        
#         indoorDust(now, t[5])
        
#         utime.sleep(1)  # Delay for 1 seconds.

# main()
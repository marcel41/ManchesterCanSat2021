# Copyright (c) 2019, Digi International, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import xbee
from machine import Pin

# Pin D9 (ON/SLEEP/DIO9)
LED_PIN_ID = "D9"
print(" +--------------------------------------+")
print(" | XBee MicroPython Receive Data Sample |")
print(" +--------------------------------------+\n")

print("Waiting for data...\n")

container_address = "0013A20041673290"
TARGET_64BIT_ADDR = b'\x00\x13\xA2\x00\x41\x67\x32\x90'
air_temperature = 28
rotation_rate = 40
altitude = 100
mock_telemetry = str(altitude) + "," + str(rotation_rate)  + "," + str(air_temperature)
MESSAGE = "Hello XBee!"

def get_add_and_msg(_send,_payload):
    addr = ("%s" % ''.join('{:02x}'.format(x).upper() for x in _send))
    msgdecode = _payload.decode()

    return addr, msgdecode

led_pin = Pin(LED_PIN_ID, Pin.OUT, value=0)
start_telemetry = True
while True:
    led_pin.value(1)
    # Check if the XBee has any message in the queue.
    received_msg = xbee.receive()
    container_msg_to_start = "transmit_data"
    if received_msg:
        sender = received_msg['sender_eui64']
        payload = received_msg['payload']
        sender_address,message_decode = get_add_and_msg(sender,payload)
        # print(message_decode)
        #print(message_decode)
        if(container_address == sender_address
          and message_decode == container_msg_to_start):
          #check the xbee again to see if a there is a new message
          cmd_decode = message_decode
          while(cmd_decode == message_decode):
            received_cmd = xbee.receive()
            if received_cmd:
              sender = received_cmd['sender_eui64']
              payload = received_cmd['payload']
              sender_address, cmd_decode = get_add_and_msg(sender,payload)
            #handle commands
            # sender_address, cmd_decode = get_add_and_msg(received_cmd)
          #print(cmd_decode)
          if cmd_decode == "Payload On":
            start_telemetry = True
          if cmd_decode == "Payload Off":
            start_telemetry = False

          if(start_telemetry):
            try:
              mock_telemetry = str(altitude) + "," + str(rotation_rate) + "," + str(air_temperature)

              xbee.transmit(TARGET_64BIT_ADDR, mock_telemetry)
              #print("Data sent successfully")
              led_pin.value(0)
            except Exception as e:
              print("Transmit failure: %s" % str(e))
        else:
          print("Transmission was not from the container")



        # print(sender_address)
        # print(message_decode)

        #container address
        # print("Data received from %s >> %s" % (''.join('{:02x}'.format(x).upper() for x in sender),
        #                                        payload.decode()))
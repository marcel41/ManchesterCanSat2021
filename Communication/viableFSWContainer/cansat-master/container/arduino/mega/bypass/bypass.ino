#include <SoftwareSerial.h>
#include <XBee.h>
#include <MemoryFree.h>


SoftwareSerial xbee_serial = SoftwareSerial(2,3);
XBee xbee = XBee();

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  // xbee_serial.begin(9600);
  // xbee.setSerial(xbee_serial);

  Serial1.begin(9600);
  xbee.setSerial(Serial1);
}

// uint8_t[4] buffer;

// the loop function runs over and over again forever
void loop() {
  while (Serial1.available() > 0) {
    Serial.write(Serial1.read());
  }
  while (Serial.available() > 0) {
      Serial1.write(Serial.read());
  }
}

void send_at_command(XBee* xbee, uint8_t* command)
{
  static AtCommandRequest at_command_request = AtCommandRequest(command);
  xbee->send(at_command_request);
  static AtCommandResponse response = AtCommandResponse();

  if (xbee->readPacket(5000)) {
    if (xbee->getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee->getResponse().getAtCommandResponse(response);

      if (response.isOk()) {
        Serial.print(response.getCommand()[0]);
        Serial.print(response.getCommand()[1]);
        Serial.println("] was successful!");
        if (response.getValueLength() > 0) {
          Serial.print("Command value length is ");
          Serial.println(response.getValueLength(), DEC);
          Serial.print("Command value: ");
          Serial.println("");
        }

      } else {
        Serial.print("Command returned error code: ");
        Serial.println(response.getStatus(), HEX);
      }
    } else {
      Serial.print("Expected AT COMMAND response but got ");
      Serial.print(xbee->getResponse().getApiId(), HEX);
    }
  } else if (xbee->getResponse().isError()) {
    Serial.print("Error reading packet.  Error code: ");
    Serial.println(xbee->getResponse().getErrorCode());
  } else {
    Serial.print("No response from radio");
  }
}

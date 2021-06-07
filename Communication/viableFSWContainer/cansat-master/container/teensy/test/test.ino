#include <SoftwareSerial.h>
#include <XBee.h>
#include <MemoryFree.h>

// SoftwareSerial xbee_serial = SoftwareSerial(2,3);
XBee xbee = XBee();

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  Serial1.begin(9600);
  xbee.setSerial(Serial1);
  pinMode(9, OUTPUT);       // reset pin gps
  digitalWrite(9, 0);
  //delay(50000);
}

// uint8_t at_command[] = {'D', 'H'};
// uint8_t at_command[] = {'I', 'D'};
// uint8_t params[] = {0x01, 0x02, 0x03, 0x04}; // Values are passed in hexadecimal
// AtCommandRequest at_command_request = AtCommandRequest(at_command, params, sizeof(params));
uint8_t idCmd2[] = {'W','R'}; // write to EEPROM

AtCommandRequest at_command_request_2 = AtCommandRequest(idCmd2);

// the loop function runs over and over again forever
void loop() {

  // Serial.print("Setting fist command");
  // xbee.send(at_command_request);
  // send_at_command();

  // at_command_request.clearCommandValue();
  // at_command_request.setCommand(idCmd2);

  // print_mem();
  Serial.print("Setting second command\n");
  xbee.send(at_command_request_2);
  send_at_command();
  Serial.print("After sending second command\n");
  while (1);
}



AtCommandResponse response = AtCommandResponse();

void send_at_command()
{
  if (xbee.readPacket(5000)) {
    if (xbee.getResponse().getApiId() == AT_COMMAND_RESPONSE) {
      xbee.getResponse().getAtCommandResponse(response);

      if (response.isOk()) {
                                  //Serial.print("I am here");
//         return;

        Serial.print("Commandjjj [");

        Serial.print(response.getCommand()[0]);
        Serial.print(response.getCommand()[1]);
        Serial.println("] was successful!");
        if (response.getValueLength() > 0) {
          Serial.print("Command value length is ");
          Serial.println(response.getValueLength(), DEC);
          Serial.print("Command value: ");
          //for (int i = 0; i < response.getValueLength(); i++) {
          //  Serial.print(response.getValue()[i], HEX);
          //  Serial.print(" ");
          //}
          Serial.println("");
        }

      } else {
        Serial.print("Command returned error code: ");
        Serial.println(response.getStatus(), HEX);
      }
    } else {
      Serial.print("Expected AT COMMAND response but got ");
      Serial.print(xbee.getResponse().getApiId(), HEX);
    }
  } else if (xbee.getResponse().isError()) {
    Serial.print("Error reading packet.  Error code: ");
    Serial.println(xbee.getResponse().getErrorCode());
  } else {
    Serial.print("No response from radio");
  }
}

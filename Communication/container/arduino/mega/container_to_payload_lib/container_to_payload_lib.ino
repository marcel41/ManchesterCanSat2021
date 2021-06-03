#include <SoftwareSerial.h>
#include <XBee.h>
//#include <MemoryFree.h>
#include "payload.h"

Payload firstPayload = Payload();

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  Serial1.begin(9600);
  firstPayload = Payload("0013A20041C8CB20", Serial1);
  String telemetryFromFirstPayload = firstPayload.send_cmd_and_receive_telemetry("Payload On");
  Serial.println(telemetryFromFirstPayload);
  delay(1000);
}

// the loop function runs over and over again forever
void loop() {

  //String text = String("2869, 01:30:57, 59, C, F, Y, Y, 103.3, 26.6, 5.02, 12:38:47, 53.4851, -2.2748, 103.3, 5, DESCEND, 54, 54, CXON");
  // String text = String("transmit_data");
  // gcs.insert_container_telemetry(text);
  //delay(2000);

  String telemetryFromFirstPayload = firstPayload.send_cmd_and_receive_telemetry("Payload On");
  Serial.println(telemetryFromFirstPayload);
  delay(1000);
}

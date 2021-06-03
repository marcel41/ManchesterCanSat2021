#include <SoftwareSerial.h>
#include <XBee.h>
//#include <MemoryFree.h>
#include "gcs.h"

GCS gcs = GCS();

bool received_cmd = false;
void command_handler(String command)
{
    Serial.print("Data was received: ");
    Serial.println(command);
    received_cmd = true;
}

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  Serial1.begin(9600);
  gcs = GCS("0013A20041C8CB20", command_handler, Serial1);
}

// the loop function runs over and over again forever
void loop() {

  received_cmd = false;
  //String text = String("2869, 01:30:57, 59, C, F, Y, Y, 103.3, 26.6, 5.02, 12:38:47, 53.4851, -2.2748, 103.3, 5, DESCEND, 54, 54, CXON");
  String text = String("transmit_data");
  gcs.insert_container_telemetry(text);
  //delay(2000);
 
  while(!received_cmd)
    gcs.process_commands();
    Serial.print("hello");
  delay(1000);
}

#include <SoftwareSerial.h>
#include <XBee.h>
#include <MemoryFree.h>
#include "gcs.h"

enum FlighState { IDLE, LAUNCHING, DESCENDING, LANDED };
class TelemetryPacket
{
  public:
    unsigned int team_id;
    String mission_time; // In UTC format
    unsigned int packet_count;
    String packet_type;
    String mode; // Flight or simulation mode
    boolean sp1_released;
    boolean sp2_released;
    unsigned int altitude;
    float temp;
    float voltage;
    String gps_time;
    double gps_latitude;
    double gps_longitude;
    double gps_altitude;
    unsigned int gps_sats;
    FlighState flight_state;
    unsigned int sp1_packet_count;
    unsigned int sp2_packet_count;
    String echo_cmd;

    String to_string();
};

String TelemetryPacket::to_string()
{
  return String(team_id) + ", " + mission_time + ", " + String(packet_count) + ", "
         + packet_type + ", " + mode + ", " + (sp1_released ? "Y" : "N") + ", " + (sp2_released ? "Y" : "N")
         + ", " + String(altitude) + ", " + String(temp) + ", " + String(voltage) + ", " + gps_time + ", "
         + String(gps_latitude) + ", " + String(gps_longitude) + ", "
         + String(gps_altitude) + ", " + String(gps_sats) + ", "
         + (flight_state == IDLE ? "IDLE" : flight_state == LAUNCHING
          ? "LAUNCHING" : flight_state == DESCENDING ? "DESCENDING" : "LANDED") + ", "
        + String(sp1_packet_count)  + ", " + String(sp2_packet_count) + ", "  + echo_cmd;
}

GCS gcs = GCS();

FlighState flight_state;

// String[] parseCommand(String string, String delimeter)
// {
//
// }
boolean send_container_telemetry;
unsigned long time_since_last_transmission;
boolean have_we_transmitted;
TelemetryPacket last_container_telemetry;

int count = 0;
bool decrease = false;
TelemetryPacket get_telemetry_data()
{
  TelemetryPacket packet;
  packet.team_id = 2869;
  packet.mission_time = "01:30:57";
  packet.packet_count = 59;
  packet.packet_type = "C";
  packet.mode = "F";
  packet.sp1_released = "Y";
  packet.sp2_released = "Y";
  // packet.altitude = 3;
  if (count < 800 && !decrease)
  {
    packet.altitude = count;
    count += 20;
  }
  else
  {
    decrease = true;
    packet.altitude = count--;
    count -= 20;
  }
  packet.temp = 6;
  packet.voltage = 02;
  packet.gps_time = "12:38:47";
  packet.gps_latitude = 4851;
  packet.gps_longitude = 2748;
  packet.gps_altitude = 3;
  packet.gps_sats = 5;
  packet.flight_state = flight_state;
  packet.sp1_packet_count = 54;
  packet.sp2_packet_count = 54;
  packet.echo_cmd = "CXON";

  // 2869, 01:30:57, 59, C, F, Y, Y, 103.3, 26.6, 5.02, 12:38:47, 53.4851, -2.2748, 103.3, 5, DESCENDING, 54, 54, CXON

  return packet;
}

void command_handler(String command)
{
    command.replace(" ", "");
    Serial.print("Data was received: ");
    Serial.println(command);
    if (command == "CMD,2869,CX,ON")
      send_container_telemetry = true;
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);

  // initialize xbee to communicate with gcs xbee
  Serial1.begin(9600);
  // gcs = GCS("0013A20040B2FFFA", command_handler, Serial1);
  gcs = GCS("0013A20041673290", command_handler, Serial1);

  // Set up initial state
  flight_state = IDLE;
  send_container_telemetry = false;
  have_we_transmitted = false;
}

void handle_transmission()
{
  unsigned long current_time = millis();
  // Send data straight away if we have not transmitted or wait for a second
  if (!have_we_transmitted || current_time > time_since_last_transmission + 1000)
  {
    // TelemetryPacket container_telemetry = get_telemetry_data();
    last_container_telemetry = get_telemetry_data();
    gcs.insert_container_telemetry(last_container_telemetry.to_string());
    // gcs.insert_container_telemetry("Que dice");
    have_we_transmitted = true;
    time_since_last_transmission = current_time;
  }
}

// the loop function runs over and over again forever
void loop() {
  // gcs.insert_container_telemetry("Que dice");
  // delay(1000);
  // return;
  // Handle commands regardless of state
  gcs.process_commands();
  switch (flight_state) {
    case IDLE:
      Serial.println("IDLE");
      if (send_container_telemetry)
        flight_state = LAUNCHING;
      break;
    case LAUNCHING:
    {
      Serial.println("LAUNCHING");
      handle_transmission();
      if (last_container_telemetry.altitude > 670)
        flight_state = DESCENDING;
      break;
    }
    case DESCENDING:
      Serial.println("DESCENDING");
      handle_transmission();
      if (last_container_telemetry.altitude < 10)
        flight_state = LANDED;
      break;
    case LANDED:
      Serial.println("LANDED");
      break;
  }
}

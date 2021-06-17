#include "telemetry.h"
// #ifndef TELEMETRY_H
// #define TELEMETRY_H
// #endif /* A_H */



String ContainerTelemetry::to_string()
{
  return String(team_id) + ", " + mission_time + ", " + String(packet_count) + ", "
         + packet_type + ", " + mode + ", " + (sp1_released ? "Y" : "N") + ", " + (sp2_released ? "Y" : "N")
         + ", " + String(altitude,1) + ", " + String(temp,1) + ", " + String(voltage,2) + ", " + gps_time + ", "
         + String(gps_latitude,4) + ", " + String(gps_longitude,4) + ", "
         + String(gps_altitude,1) + ", " + String(gps_sats) + ", "
         + (flight_state == IDLE ? "IDLE" : flight_state == LAUNCHING
          ? "LAUNCHING" : flight_state == DESCENDING ? "DESCENDING" : "LANDED") + ", "
        + String(sp1_packet_count)  + ", " + String(sp2_packet_count) + ", "  + echo_cmd;
}

String PayloadTelemetry::to_string()
{
  return String(team_id) + ", " + mission_time + ", " + String(packet_count) + ", "
         + packet_type + ", " + String(altitude,1) + ", " + String(temp,1) + ", "
         + String(sp_rotation);
}

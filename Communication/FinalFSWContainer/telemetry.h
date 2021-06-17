#ifndef TELEMETRY_H
#define TELEMETRY_H


#include <WString.h>
enum FlighState { IDLE, LAUNCHING, DESCENDING, LANDED };

class PayloadTelemetry
{
  public:
    unsigned int team_id;
    String mission_time; // In UTC format
    long packet_count;
    String packet_type;
    float altitude;
    float temp;
    unsigned int sp_rotation;

    String to_string();
};

class ContainerTelemetry
{
  public:
    unsigned int team_id;
    String mission_time; // In UTC format
    long packet_count;
    String packet_type;
    String mode; // Flight or simulation mode
    bool sp1_released;
    bool sp2_released;
    float altitude;
    float temp;
    float voltage;
    String gps_time;
    double gps_latitude;
    double gps_longitude;
    double gps_altitude;
    unsigned int gps_sats;
    FlighState flight_state;
    long sp1_packet_count;
    long sp2_packet_count;
    String echo_cmd;

    String to_string();
};

#endif

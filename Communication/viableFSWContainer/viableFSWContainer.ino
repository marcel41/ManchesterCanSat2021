#include <TinyGPS++.h>
#include <Servo.h>
#include <Adafruit_DPS310.h>
#include <math.h>
#include "gcs.h"
#include "payload.h"
#include <TimeLib.h>
#include <XBee.h>
#define SerialMonitor Serial
#define SerialGPS Serial2



#define GPS_BAUD 9600 // GPS module baud rate. GP3906 defaults to 9600
#define  PIN_D6 6
#define  PIN_D5 5
#define  Voltage_Battery 15

TinyGPSPlus tinyGPS; // Create a TinyGPSPlus object
Adafruit_DPS310 dps;
Servo myservo;

#define ARDUINO_SERVO_PIN 17 // GPS TX, Teensy RX pin

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
    float altitude;
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

class TelemetryPacketPayload
{
  public:
    unsigned int team_id;
    String mission_time; // In UTC format
    unsigned int packet_count;
    String packet_type;
    float altitude;
    float temp;
    float sp_rotation;

    String to_string();
};

String TelemetryPacket::to_string()
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


GCS gcs = GCS();
Payload firstPayload = Payload();
Payload secondPayload = Payload();

FlighState flight_state;
boolean send_container_telemetry;
unsigned long time_since_last_transmission;
boolean have_we_transmitted;
TelemetryPacket last_container_telemetry;

boolean releasePayload1 = false;
boolean releasePayload2 = false;
int count = 0;
bool decrease = false;

String GPStime;
double GPSlatitude;
double GPSlongitude;
double GPSaltitude;
boolean sp1_released;
boolean sp2_released;
float calculatedAltitude;
float tempCalculated;
float voltageCalculated;
unsigned int GPSsats;
unsigned int SP1packet_count = 0;
unsigned int SP2packet_count = 0;
String echoCMD;
String timeOfMission = "";
float rounding(float valor, int decimales) {
   double _potencia = pow(10, decimales);
   return (roundf(valor * _potencia) / _potencia);
 };





 String TelemetryPacketPayload::to_string()
 {
   return String(team_id) + ", " + mission_time + ", " + String(packet_count) + ", "
          + packet_type + ", " + String(altitude,1) + ", " + String(temp,1) + ", "
          + String(sp_rotation);
 }


 float p1_altitude;
 float p1_temp;
 int p1_sp_rotation;


TelemetryPacketPayload get_telemetry_data_payload1()
{
  TelemetryPacketPayload packet;
  packet.team_id = 2869;
  packet.mission_time = timeOfMission;
  packet.packet_count = SP1packet_count;
  packet.packet_type = "S1";
  packet.altitude = p1_altitude;
  packet.temp = p1_temp;
  packet.sp_rotation = p1_sp_rotation;

  return packet;
}

float p2_altitude;
float p2_temp;
float p2_sp_rotation;


TelemetryPacketPayload get_telemetry_data_payload2()
{
  TelemetryPacketPayload packet;
  packet.team_id = 2869;
  packet.mission_time = timeOfMission;
  packet.packet_count = SP2packet_count;
  packet.packet_type = "S2";
  packet.altitude = p2_altitude;
  packet.temp = p2_temp;
  packet.sp_rotation = p2_sp_rotation;

  return packet;
}

// TelemetryPacketPayload get_telemetry_data_payload2()
// {
//    return null
// }
void printDigits(int digits,char leadingZero='0');
TelemetryPacket get_telemetry_data_container()
{
  TelemetryPacket packet;
  packet.team_id = 2869;
  packet.mission_time = timeOfMission;
  packet.packet_count = count;
  packet.packet_type = "C";
  packet.mode = "F";
  packet.sp1_released = releasePayload1;
  packet.sp2_released = releasePayload2;
  packet.temp = tempCalculated;
  packet.altitude = calculatedAltitude;
  packet.voltage = voltageCalculated;
  packet.gps_time = GPStime;
  packet.gps_latitude = GPSlatitude;
  packet.gps_longitude = GPSlongitude;
  packet.gps_altitude = GPSaltitude;
  packet.gps_sats = GPSsats;
  packet.flight_state = flight_state;
  packet.sp1_packet_count = SP1packet_count;
  packet.sp2_packet_count = SP2packet_count;
  packet.echo_cmd = "CXON";
  // 2869, 01:30:57, 59, C, F, Y, Y, 103.3, 26.6, 5.02, 12:38:47, 53.4851, -2.2748, 103.3, 5, DESCENDING, 54, 54, CXON
  return packet;
}


// This custom version of delay() ensures that the tinyGPS object
// is being "fed". From the TinyGPS++ examples.
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (SerialGPS.available())
    {
      // char data_received = SerialGPS.read()
      // Serial.println(data_received);
      tinyGPS.encode(SerialGPS.read()); // Send it to the encode function
    }
    // tinyGPS.encode(char) continues to "load" the tinGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}


/********************************************************************************************************************
 **** Alt *******************************************************************************************
 ********************************************************************************************************************
*/

//set up all the variables need for sensors

/********************************************************************************************************************
 **** Time check (&format) *******************************************************************************************
 ********************************************************************************************************************
*/
// unsigned long checkTime() {
//   if (RTCFunctional) {
//     return (rtc.now().secondstime() - missionStartTime);
//   }
//   else {
//     return (EEPROMTime + millis()) / 1000;
//   }
// }


// const float seaLevelPressureLevel = 1013.25;
const float seaLevelPressureLevel = 1023.0;
float getAlt(float newPressure,float newTemperature) {
  // previousAlt = alt;
  return ((newTemperature + 273.15) / 0.0065) * (pow(seaLevelPressureLevel/newPressure, 0.1903) -1);
}


void releaseMechanism()
{
  if(last_container_telemetry.altitude <= 500 && !releasePayload1)
  {
      myservo.write(75 + 40);
      releasePayload1 = true;
  }
  else if(last_container_telemetry.altitude <= 400 && !releasePayload2)
  {
    myservo.write(75 - 40);
    releasePayload2 = true;
  }
}


void readSensors()
{

  sensors_event_t temp_event, pressure_event;
  //read temperature


  while (!dps.temperatureAvailable() || !dps.pressureAvailable()) {
    continue;
  }
  dps.getEvents(&temp_event, &pressure_event);
  tempCalculated = temp_event.temperature;
  // Serial.println(pressure_event.pressure);
  // Serial.println(dps.readAltitude(1023));
  calculatedAltitude = getAlt(pressure_event.pressure,tempCalculated);



  smartDelay(100);

  GPSaltitude = tinyGPS.altitude.meters();
  GPSlatitude = tinyGPS.location.lat();
  GPSlongitude = tinyGPS.location.lng();
  GPSsats = tinyGPS.satellites.value();
  GPStime = "";
  GPStime = GPStime + String(tinyGPS.time.hour()) + ":" ;
  if (tinyGPS.time.minute() < 10)
    GPStime = GPStime + "0";
  GPStime = GPStime + String(tinyGPS.time.minute()) + ":" ;
  if (tinyGPS.time.second() < 10)
    GPStime = GPStime + "0";
  GPStime = GPStime + String(tinyGPS.time.second());

  voltageCalculated = analogRead(Voltage_Battery) * (3.3 / 1023) * 0.8734;

  timeOfMission = utc_format(Teensy3Clock.get());
}

void setUpSensors()
{

  //DPS310 Testing
  // SerialMonitor.begin(115200);
  // while (!SerialMonitor) delay(10);

  //Look for the sensors
  // SerialMonitor.println("DPS310");


  if (! dps.begin_I2C()) {             // Can pass in I2C address here
    //if (! dps.begin_SPI(DPS310_CS)) {  // If you want to use SPI
      // SerialMonitor.println("Failed to find DPS");
      while (1) yield();
    }
    // SerialMonitor.println("DPS OK!");

    dps.configurePressure(DPS310_16HZ, DPS310_16SAMPLES);
    dps.configureTemperature(DPS310_16HZ, DPS310_16SAMPLES);







  //GPS Set up
  // Serial.println("Checking GPS");
  pinMode(PIN_D6, OUTPUT);       // interrupt pin in gps
  pinMode(PIN_D5, OUTPUT);       // reset pin gps
  digitalWrite(PIN_D5, LOW);    // reset
  delay(250);
  digitalWrite(PIN_D5, HIGH);   //
  digitalWrite(PIN_D6, LOW);   // no interrupt
  SerialGPS.begin(GPS_BAUD);
  while (!SerialGPS.available()) continue; //until getting gps

  // Serial.println("GPS is OK!");
  //Set up the voltage reading
  pinMode(Voltage_Battery, INPUT); //analog reading
  //set up rtc

  // if (! rtc.begin()) {
  //   Serial.println("Couldn't find RTC");
  //   Serial.flush();
  //   abort();
  // }
  //
  // if (! rtc.isrunning()) {
  //   Serial.println("RTC is NOT running, let's set the time!");
  //
  //   DateTime dateStart = new DateTime((2021, 06, 06, 13, 0, 0))
  //   rtc.adjust(dateStart);
  // }
  //Set start mission with 0, 0, 0
  setTime(0,0,0,06, 06, 2021);
  Teensy3Clock.set(now());
  // Serial.print("Initial RTC:     "); digitalClockDisplay(Teensy3Clock.get());


  //Rtc testing
}

void digitalClockDisplay(time_t x) {
  // printDigits(hour(x),' ');
  // Serial.print(":");
  // printDigits(minute(x));
  // Serial.print(":");
  // printDigits(second(x));
  // Serial.print(" ");
  // printDigits(day(x),' ');
  // Serial.print(" ");
  // printDigits(month(x),' ');
  // Serial.print(" ");
  // Serial.println(year(x));
  ;
}

String utc_format(time_t x)
{
  String utc_time = String(hour(x)) + ":";
  if(int(minute(x)) < 10)
    utc_time = utc_time + "0";
  utc_time = utc_time + String(minute(x)) + ":";
  if(int(second(x)) < 10)
    utc_time = utc_time + "0";
  utc_time = utc_time + String(second(x));

  return utc_time;
}

void printDigits(int digits,char leadingZero){
  // utility function for digital clock display: prints preceding colon and leading 0
  // if(digits < 10)
  //   Serial.print(leadingZero);
  // Serial.print(digits);
  ;
}

void command_handler(String command)
{
    command.replace(" ", "");
    // Serial.print("Data was received: ");
    // Serial.println(command);
    if (command == "CMD,2869,CX,ON")
      send_container_telemetry = true;
}


void setup() {

  // initialize xbee to communicate with gcs xbee
  Serial.begin(9600);
  Serial1.begin(9600);
  // Serial.println("Enter");
  firstPayload = Payload("0013A20041C8CBB9", Serial1);
  // secondPayload = Payload("0013A20041C8CB20", Serial);
  gcs = GCS("0013A20041673290", command_handler, Serial1);

  // Set up initial state
  // flight_state = IDLE;
  flight_state = LAUNCHING;
  send_container_telemetry = false;
  have_we_transmitted = false;

  myservo.attach(ARDUINO_SERVO_PIN);
  myservo.write(75);  // set servo to mid-point
  setUpSensors();


  // String telemetryFromFirstPayload = firstPayload.send_cmd_and_receive_telemetry("Payload On");
  // while (telemetryFromFirstPayload == "No message")
  // {
  //   telemetryFromFirstPayload = firstPayload.send_cmd_and_receive_telemetry("Payload On");
  //
  //   Serial.println("Not Synchronised");
  // }
  // Serial.println("Synchronised");
}


// std::vector<String> splitStringToVector(String msg){
//   std::vector<String> subStrings;
//   int j=0;
//   for(int i =0; i < msg.length(); i++){
//     if(msg.charAt(i) == ','){
//       subStrings.push_back(msg.substring(j,i));
//       j = i+1;
//     }
//   }
//   subStrings.push_back(msg.substring(j,msg.length())); //to grab the last value of the string
//   return subStrings;
// }

//parse the info from the payload
String stringToArrayString(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void handle_transmission()
{
  unsigned long current_time = millis();
  // Send data straight away if we have not transmitted or wait for a second
  if (!have_we_transmitted || current_time > time_since_last_transmission + 1000)
  {
      count++;
      last_container_telemetry = get_telemetry_data_container();
      gcs.insert_container_telemetry(last_container_telemetry.to_string());

      //
      // String telemetryFromFirstPayload = "No message";
      // telemetryFromFirstPayload = firstPayload.send_cmd_and_receive_telemetry("Payload On");
      // // Serial.println(telemetryFromFirstPayload);
      //
      // if(telemetryFromFirstPayload != "No message")
      // {
      //   String first = stringToArrayString(telemetryFromFirstPayload, ',', 0);
      //   String second = stringToArrayString(telemetryFromFirstPayload, ',', 1);
      //   String third = stringToArrayString(telemetryFromFirstPayload, ',', 2);
      //   //
      //   // Serial.println(first);
      //   // Serial.println(second);
      //   // Serial.println(third);
      //
      //   p1_temp = first.toFloat();
      //   p1_altitude = second.toInt();
      //   p1_sp_rotation = third.toInt();
      //   SP1packet_count++;
      //
      //   TelemetryPacketPayload last_container_telemetry = get_telemetry_data_payload1();
      //   gcs.insert_container_telemetry(last_container_telemetry.to_string());
      // }
      // Serial.println(last_container_telemetry.to_string());

      have_we_transmitted = true;
      time_since_last_transmission = current_time;
    }

/*
    if(releasePayload2 == true)
    {
      String telemetryFromSecondPayload = secondPayload.send_cmd_and_receive_telemetry("Payload On");
      std::vector<String> telemetry_array = splitStringToVector(telemetryFromSecondPayload);
      p1_altitude = telemetry_array[0];
      p1_temp = telemetry_array[1];
      p1_sp_rotation = telemetry_array[2];
      last_container_telemetry = get_telemetry_data_payload2();
      gcs.insert_container_telemetry(last_container_telemetry.to_string());
    }
*/


}


void loop() {
  // if(have_we_transmitted == false)
  gcs.process_commands();

  // String telemetryFromFirstPayload = firstPayload.send_cmd_and_receive_telemetry("Payload On");
  // // String telemetryFromFirstPayload = firstPayload.send_cmd_and_receive_telemetry("Payload On");
  // Serial.println("working");
  // Serial.println(telemetryFromFirstPayload);
  // Serial.println("no working");
  //
  // delay(1000);
  // return;

  switch (flight_state) {
    case IDLE:
      // Serial.println("IDLE");
      if (send_container_telemetry)
      {


        // String telemetryFromFirstPayload = firstPayload.send_cmd_and_receive_telemetry("Payload On");
        // // String telemetryFromFirstPayload = firstPayload.send_cmd_and_receive_telemetry("Payload On");
        // Serial.println("working");
        // Serial.println(telemetryFromFirstPayload);
        // Serial.println("no working");

        // delay(1000);
        // return;

        flight_state = LAUNCHING;
        // while (1);
      }
      break;
    case LAUNCHING:
    {
      // Serial.println("LAUNCHING");
      readSensors();
      handle_transmission();
      if (last_container_telemetry.altitude > 670)
        flight_state = DESCENDING;
      break;
    }
    case DESCENDING:
      // Serial.println("DESCENDING");
      readSensors();
      handle_transmission();
      releaseMechanism();
      if (last_container_telemetry.altitude < 10)
        flight_state = LANDED;
      break;
    case LANDED:
      // Serial.println("LANDED");
      while(1)
        continue;
  }
  // //wait for a second to do all again
  // while(millis() - start_second < 1000)
  //   continue;

}

#include <SoftwareSerial.h>
#include <XBee.h>
#include "gcs.h"
#include "payload.h"
#include "telemetry.h"

#include <TinyGPS++.h>
#include <Servo.h>
#include <Adafruit_DPS310.h>
#include <math.h>
#include <TimeLib.h>
#include "SparkFunLSM6DS3.h"
#define SerialMonitor Serial
#define SerialGPS Serial2



//pin definitions---------------------------------------------------------------

#define GPS_BAUD 9600 // GPS module baud rate. GP3906 defaults to 9600
#define  PIN_D6 6
#define  PIN_D5 5
#define  Voltage_Battery 15
LSM6DS3 myIMU; //Default constructor is I2C, addr 0x6B
TinyGPSPlus tinyGPS; // Create a TinyGPSPlus object
Adafruit_DPS310 dps;
Servo myservo;

#define ARDUINO_SERVO_PIN 17 // GPS TX, Teensy RX pin
//pin definitions---------------------------------------------------------------



GCS gcs = GCS();

Payload payload_1 = Payload();

FlighState flight_state;

boolean send_container_telemetry;
unsigned long time_since_last_transmission;
boolean have_we_transmitted;
ContainerTelemetry last_container_telemetry;

long count = 0;
bool decrease = false;


String GPStime = "";
double GPSlatitude;
double GPSlongitude;
double GPSaltitude;

boolean releasePayload1 = false;
boolean releasePayload2 = false;
boolean sim_enable = false;
boolean sim_activate = false;
float airpressure_container_sim = 1023.0;
// boolean ack_simulation = false;
// boolean ack_simulation = false;

float calculatedAltitude;
float tempCalculated;
float voltageCalculated;
unsigned int GPSsats;
long SP1packet_count = 0;
long SP2packet_count = 0;
String echoCMD = "CXON";
String timeOfMission = "";
String modeSimOrFlight = "F";

ContainerTelemetry get_container_telemetry()
{
  ContainerTelemetry packet;
  packet.team_id = 2869;
  // packet.mission_time = "01:30:57";
  packet.mission_time = timeOfMission;
  packet.packet_count = count;
  packet.packet_type = "C";
  packet.mode = modeSimOrFlight;
  packet.sp1_released = releasePayload1;
  packet.sp2_released = releasePayload2;
  // packet.altitude = 3;
  // if (count < 800 && !decrease)
  // {
  //   packet.altitude = count;
  //   count += 20;
  // }
  // else
  // {
  //   decrease = true;
  //   packet.altitude = count--;
  //   count -= 20;
  // }
  packet.altitude = calculatedAltitude;
  packet.temp = tempCalculated;
  packet.voltage = voltageCalculated;
  packet.gps_time = GPStime;
  packet.gps_latitude = GPSlatitude;
  packet.gps_longitude = GPSlongitude;
  packet.gps_altitude = GPSaltitude;
  packet.gps_sats = GPSsats;
  packet.flight_state = flight_state;
  packet.sp1_packet_count = SP1packet_count;
  packet.sp2_packet_count = SP2packet_count;
  packet.echo_cmd = echoCMD;

  return packet;
}
void printDigits(int digits,char leadingZero='0');


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

// const float seaLevelPressureLevel = 1013.25;
const float seaLevelPressureLevel = 1023.0; //at Manchester
float getAlt(float newPressure,float newTemperature) {
  // previousAlt = alt;
  return ((newTemperature + 273.15) / 0.0065) * (pow(seaLevelPressureLevel/newPressure, 0.1903) -1);
}


void releaseMechanism()
{
  if(last_container_telemetry.altitude <= 500 && !releasePayload1)
  {
      myservo.write(75 + 40); //place on the left side payload ref container pcb
      releasePayload1 = true;
  }
  else if(last_container_telemetry.altitude <= 400 && !releasePayload2)
  {
    myservo.write(75 - 40); //right side for payload ref container pcb
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
  Serial.println("temp: " + String(tempCalculated));
  if(sim_enable && sim_activate)
    calculatedAltitude = getAlt(airpressure_container_sim,tempCalculated);
  else
    calculatedAltitude = getAlt(pressure_event.pressure,tempCalculated);
  Serial.println("press: " + String(pressure_event.pressure));
  Serial.println("alt: " + String(calculatedAltitude));



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

  voltageCalculated = analogRead(Voltage_Battery) * (3.3 / 1023);

  timeOfMission = utc_format(Teensy3Clock.get());
}


void setUpSensors()
{

  //DPS310 Testing
  // SerialMonitor.begin(115200);
  // while (!SerialMonitor) delay(10);

  //Look for the sensors
  // SerialMonitor.println("DPS310");
  //set up the rate of imu
  // myIMU.settings.gyroSampleRate = 1666; default ~446
  //start of imu
  myIMU.begin();

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

  //Set up the voltage reading
  pinMode(Voltage_Battery, INPUT); //analog reading

  //rtc setting up
  setTime(0,0,0,06, 06, 2021);
  Teensy3Clock.set(now());
}


String utc_format(time_t x)
{
  String utc_time = "";
  if(int(hour(x)) < 10)
    utc_time = utc_time + "0";
  utc_time = utc_time + String(hour(x)) + ":";
  if(int(minute(x)) < 10)
    utc_time = utc_time + "0";
  utc_time = utc_time + String(minute(x)) + ":";
  if(int(second(x)) < 10)
    utc_time = utc_time + "0";
  utc_time = utc_time + String(second(x));

  return utc_time;
}




String string_to_array_container(String data, char separator, int index)
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


void command_handler(String command)
{
    command.replace(" ", "");
    Serial.print("Data was received: ");
    Serial.println(command);
    if(command.length() > 4)
    {
      //SOMEHOW payload collides here
      String firstPartOfCMD = string_to_array_container(command,',',0);
      if(firstPartOfCMD == "CMD")
      {
        String firstPartOfCMD = string_to_array_container(command,',',2);
        String secondPartOfCMD = string_to_array_container(command,',',3);
        if (command == "CMD,2869,CX,ON")
          send_container_telemetry = true;
        else if(command == "CMD,2869,CX,ON")
          send_container_telemetry = false;
        else if(command == "CMD,2869,SIM,ENABLE")
          sim_enable = true;
        else if(command == "CMD,2869,SIM,ACTIVATE")
          sim_activate = true;
        else if(command == "CMD,2869,SIM,DISABLE")
          sim_enable = false;
        else if(firstPartOfCMD == "ST")
        {
          //rtc setting up
          //get the hour, min and second
          int hour_set = string_to_array_container(secondPartOfCMD,':',0).toInt();
          int min_set = string_to_array_container(secondPartOfCMD,':',1).toInt();
          int sec_set = string_to_array_container(secondPartOfCMD,':',2).toInt();
          setTime(hour_set,min_set,sec_set,06, 06, 2021);
          Teensy3Clock.set(now());
        }
        echoCMD = firstPartOfCMD + secondPartOfCMD;

      }
    }
    // echoCMD = string_to_array_container(command,",",0);
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);

  // initialize xbee to communicate with gcs xbee
  Serial1.begin(9600);
  gcs = GCS("0013A20041673290", command_handler, Serial1);
  // gcs = GCS("0013A20041673290", command_handler, Serial1);

  // Set up initial state
  flight_state = LAUNCHING;
  // flight_state = IDLE;
  send_container_telemetry = false;
  have_we_transmitted = false;

  payload_1 = Payload("0013A20041C8CBB9", Serial1);

  while (!payload_1.connected())
  {
    Payload::update();
    Serial.println("Trying to connect");
    delay(1000);
  }

  Serial.println("Connected");
  myservo.attach(ARDUINO_SERVO_PIN);
  myservo.write(70);  // set servo to mid-point
  setUpSensors();
}

void handle_transmission()
{
  unsigned long current_time = millis();
  // Send data straight away if we have not transmitted or wait for a second
  if (!have_we_transmitted || current_time > time_since_last_transmission + 1000)
  {
    // TO DO: Switch to gcs net id
    // Process gcs commands
    gcs.process_commands();

    // Insert container telemetry
    count++;
    last_container_telemetry = get_container_telemetry();
    gcs.insert_container_telemetry(last_container_telemetry.to_string());
    have_we_transmitted = true;
    time_since_last_transmission = current_time;
    releasePayload1 = true;
    if(releasePayload1 == true)
    {
      // Insert payload telemetry
      // sp1_packet_count++;
      PayloadTelemetry payload_1_telemetry = payload_1.get_telemetry();
      // All the other fields are set except for these
      payload_1_telemetry.team_id = last_container_telemetry.team_id;
      payload_1_telemetry.mission_time = last_container_telemetry.mission_time;
      payload_1_telemetry.packet_count = count + 1;
      count++;
      SP1packet_count++;
      gcs.insert_payload_telemetry(payload_1_telemetry.to_string());

    }

    if(releasePayload2 == true)
    {
      //Payload 2 will not be necessary
      // SP2packet_count
      ;
    }

  }
}

// the loop function runs over and over again forever
void loop() {
  // Handle payload regardless of state
  Payload::update();
  switch (flight_state) {
    case IDLE:
      Serial.println("IDLE");
      // Handle commands Continuously
      // May be costly cause it is constantly requesting commands
      gcs.process_commands();
      if (send_container_telemetry)
        flight_state = LAUNCHING;
      break;
    case LAUNCHING:
    {
      Serial.println("LAUNCHING");
      readSensors();
      handle_transmission();
      if (last_container_telemetry.altitude > 670)
        flight_state = DESCENDING;
      break;
    }
    case DESCENDING:
      Serial.println("DESCENDING");
      readSensors();
      handle_transmission();
      releaseMechanism();
      if (last_container_telemetry.altitude < 10)
        flight_state = LANDED;
      break;
    case LANDED:
      //buzzer code is missing place here
      Serial.println("LANDED");
      break;
  }
}

#include "payload.h"

// ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// Telemetry::Telemetry(){}
// static void Telemetry::setTelemetry(String s)
// {
//   Telemetry::telemetryData = s;
// }
// static String Telemetry::getTelemetry(){
//   return Telemetry::telemetryData;
// }


Payload::Payload(){}

Payload::Payload(String addr64, Stream &serial)
{
  // Take the last 8 characters as Low address
  String first_half = addr64.substring(addr64.length() - 8, addr64.length());
  // Take the rest as high address
  String second_half = addr64.substring(0, addr64.length() - 8);
  _addr64 = XBeeAddress64(strtoul(second_half.c_str(), NULL, 16),
                          strtoul(first_half.c_str(), NULL, 16));
  _serial = &serial;
  _xbee.setSerial(*_serial);
  // telemetry = Telemetry();
  _xbee.onZBRxResponse(telemetry_handler_wrapper);
  _isPayloadDetected = false;
  _telemetry_from_xbee = "";
  _isPayloadTransmitting = false;
  txStatus = ZBTxStatusResponse();
  rxResponse = ZBRxResponse();
  response = XBeeResponse();
}
unsigned long time_Start;
bool Payload::checkForSuccess()
{
  bool sucess_trans = false;

  // the chose timeout is going to be 100ms in AT configuration
  // average 22 chracters around 30 ms

  //the chose timout in API mode 2 is 10ms, no prints in the payloads
  //average 22 chracters around 1ms
  if (_xbee.readPacket(100))
  {
    // got a response!
    // should be a znet tx status

    if (_xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      _xbee.getResponse().getZBTxStatusResponse(txStatus);

      if (txStatus.getDeliveryStatus() == SUCCESS) {
        unsigned long time_elapse = millis() - time_Start;
        Serial.println("SUCCESS" + String(time_elapse));
        sucess_trans = true;
      } else {
        Serial.println("Xbee might not be powered on");
      }
    }
  }
  else if (_xbee.getResponse().isError())
  {
    Serial.print("ERROR");
    //nss.print("Error reading packet.  Error code: ");
    //nss.println(_xbee.getResponse().getErrorCode());
  }
  else
  {
    // local _xbee did not provide a timely TX Status Response -- should not happen
    Serial.println("xbee did not provide a TX status response");
  }
  return sucess_trans;
}


int payload_number = 0; //delete for deployment
void Payload::connectToPayload()
{
  _isPayloadDetected = false;
  unsigned long last_time = millis();
  unsigned long miliseconds_limit = last_time + 500;

  // String request_message = "transmit_data" + String(payload_number);
  String request_message = "transmit_data";
  payload_number = payload_number + 1;

  while(miliseconds_limit > last_time)
  {
    // Serial.println("Connecting to payload");
    // Serial.println("---------------------");
    //
    // Serial.println("Sending request from container");
    ZBTxRequest zbTx = ZBTxRequest(_addr64, request_message.c_str(), request_message.length());
    _xbee.send(zbTx);
    time_Start = millis();
    // Serial.println("---------------------");

    //after sending the request, it is necessary
    //to check for a status response in order to
    //check the container and the xbee is not sleep
    _isPayloadDetected = checkForSuccess();
    if(_isPayloadDetected)
      break;
    // Serial.println("Retrying connection ...");
    last_time = millis();
  }
}

// String Payload::send_cmd_and_receive_telemetry(String cmd)
// {
//
//   _telemetry_from_xbee = "No message";
//   connectToPayload();
//   if(_isPayloadDetected)
//   {
//     //send commands
//     Serial.println("Sending data");
//     ZBTxRequest zbTx = ZBTxRequest(_addr64, cmd.c_str(), cmd.length());
//     _xbee.send(zbTx);
//     time_Start = millis();
//     if(checkForSuccess())
//     {
//       // Serial.println("Command was sent from container to payload");
//       if(cmd == "Payload On")
//       {
//         _isPayloadTransmitting = true;
//         // Serial.println("Transmission is happening");
//       }
//       else if(cmd == "Payload Off")
//         _isPayloadTransmitting = false;
//     }
//     else
//       Serial.println("Command was not sent, although payload was found");
//
//
//     if(_isPayloadTransmitting == true)
//     {
//       // Serial.println("The fun");
//       unsigned long last_time = millis();
//       unsigned long start_t = millis();
//
//       unsigned long miliseconds_limit = last_time + 200;
//       while(miliseconds_limit > last_time)
//       {
//         //same as the reading, average 50ms for 11 characters
//         if(_xbee.readPacket(3000));
//         {
//           if (_xbee.getResponse().isAvailable()) {
//             Serial.println("There's some RX.");
//
//             if (_xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
//               Serial.println("\tThere's RX packet.");
//               _xbee.getResponse().getZBRxResponse(rxResponse);
//
//               char buffer[rxResponse.getDataLength() + 1];
//               memcpy(buffer, rxResponse.getFrameData() + rxResponse.getDataOffset(), rxResponse.getDataLength());
//               buffer[rxResponse.getDataLength()] = '\0';
//
//               _telemetry_from_xbee = String(buffer);
//
//               unsigned long elapsed_time = millis();
//               elapsed_time = elapsed_time - start_t;
//               // Serial.println(_telemetry_from_xbee);
//               // Serial.println(elapsed_time);
//               break;
//             }
//           }
//         }
//         last_time = millis();
//       }
//     }
//   }
//   else
//   {
//     Serial.println("Container did not find the xbee, skipping command");
//   }
//   return _telemetry_from_xbee;
// }


String Payload::send_cmd_and_receive_telemetry(String cmd)
{

  _telemetry_from_xbee = "No message";
  connectToPayload();
  if(_isPayloadDetected)
  {
    //send commands
    // Serial.println("Sending data");
    ZBTxRequest zbTx = ZBTxRequest(_addr64, cmd.c_str(), cmd.length());
    _xbee.send(zbTx);
    time_Start = millis();
    if(checkForSuccess())
    {
      // Serial.println("Command was sent from container to payload");
      if(cmd == "Payload On")
      {
        _isPayloadTransmitting = true;
        // Serial.println("Transmission is happening");
      }
      else if(cmd == "Payload Off")
        _isPayloadTransmitting = false;
    }
    else
      // Serial.println("Command was not sent, although payload was found");
      ;

    if(_isPayloadTransmitting == true)
    {
      // Serial.println("The fun");
      unsigned long last_time = millis();
      unsigned long start_t = millis();

      unsigned long miliseconds_limit = last_time + 200;
      while(miliseconds_limit > last_time)
      {
        //same as the reading, average 50ms for 11 characters
        if(_xbee.readPacket(500));
        {
          if (_xbee.getResponse().isAvailable()) {
            // Serial.println("There's some RX.");

            if (_xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
              // Serial.println("\tThere's RX packet.");
              _xbee.getResponse().getZBRxResponse(rxResponse);

              char buffer[rxResponse.getDataLength() + 1];
              memcpy(buffer, rxResponse.getFrameData() + rxResponse.getDataOffset(), rxResponse.getDataLength());
              buffer[rxResponse.getDataLength()] = '\0';

              _telemetry_from_xbee = String(buffer);

              unsigned long elapsed_time = millis();
              elapsed_time = elapsed_time - start_t;
              // Serial.println(_telemetry_from_xbee);
              // Serial.println(elapsed_time);
              break;
            }
          }
        }
        last_time = millis();
      }
    }
  }
  else
  {
    // Serial.println("Container did not find the xbee, skipping command");
    ;
  }
  return _telemetry_from_xbee;
}




// void Payload::setUpTelemetry(int telemetryP)
// {
//   _telemetry_from_xbee = telemetryP;
// }

XBeeWithCallbacks Payload::_xbee;
// void convert
// Payload::_telemetry_from_xbee = "";
void Payload::telemetry_handler_wrapper(ZBRxResponse& rx, uintptr_t) {
  // Serial.println("INSIDE telemetry_handler_wrapper");

    char buffer[rx.getDataLength() + 1];
    memcpy(buffer, rx.getFrameData() + rx.getDataOffset(), rx.getDataLength());
    buffer[rx.getDataLength()] = '\0';
    // _telemetry_from_xbee = String(buffer);
    String string_from_payload = String(buffer);
    // setUpTelemetry(string_from_payload);
    // telemetry.setTelemetry(string_from_payload);
    Serial.println(string_from_payload);
}

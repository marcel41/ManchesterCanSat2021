#include "payload.h"
Payload::Payload(){
  // Append to array
  payloads[count++] = this;
}

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
  _xbee.onZBRxResponse(on_rx_response);
  _connected = false;
}

void Payload::update()
{
  _xbee.loop();
}

bool Payload::check_for_success()
{
  ZBTxStatusResponse txStatus;

  bool sucess_trans = false;

  // the chose timeout is going to be 100ms in AT configuration
  // average 22 chracters around 30 ms

  //the chose timout in API mode 2 is 10ms, no prints in the payloads
  //average 22 chracters around 1ms
  if (_xbee.readPacket(400))
  {
    // got a response!
    // should be a znet tx status
    if (_xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      _xbee.getResponse().getZBTxStatusResponse(txStatus);

      if (txStatus.getDeliveryStatus() == SUCCESS) {
        Serial.println("SUCCESS");
        sucess_trans = true;
      } else {
        Serial.println("Xbee might not be powered on" + String(txStatus.getDeliveryStatus(), HEX));
      }
    }
  }
  else if (_xbee.getResponse().isError())
  {
    Serial.print("ERROR");
  }
  else
  {
    // local _xbee did not provide a timely TX Status Response -- should not happen
    Serial.println("xbee did not provide a TX status response");
  }
  return sucess_trans;
}

void Payload::send_data(String data)
{
  Serial.println("Sending data: " + data);
  ZBTxRequest zbTx = ZBTxRequest(_addr64, (uint8_t*) data.c_str(), data.length());
  _xbee.send(zbTx);

  // TODO: Check for success
  check_for_success();
}

bool Payload::connected()
{
  return _connected;
}

XBeeWithCallbacks Payload::_xbee;

Payload* Payload::payloads[2] = {NULL, NULL};
int Payload::count = 0;

void Payload::on_rx_response(ZBRxResponse& rx, uintptr_t) {
  char buffer[rx.getDataLength() + 1];
  memcpy(buffer, rx.getFrameData() + rx.getDataOffset(), rx.getDataLength());
  buffer[rx.getDataLength()] = '\0';
  String message = String(buffer);

  Payload* from;
  if (message.startsWith("S1"))
    from = payloads[0];
  else if (message.startsWith("S2"))
    from = payloads[1];
  else
  {
    Serial.println("Something is wrong. Unexpected message");
    Serial.println(message);
    return;
  }

  from->_connected = true;
  PayloadTelemetry new_telemetry;

  new_telemetry.packet_type = string_to_array_string(message, ',', 0);
  new_telemetry.altitude = string_to_array_string(message, ',', 1).toFloat();
  new_telemetry.temp = string_to_array_string(message, ',', 2).toFloat();
  new_telemetry.sp_rotation = string_to_array_string(message, ',', 3).toInt();

  from->_latest_telemetry = new_telemetry;

  Serial.println("From payload " + new_telemetry.to_string());
}

//parse the info from the payload
String Payload::string_to_array_string(String data, char separator, int index)
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


PayloadTelemetry Payload::get_telemetry()
{
  return _latest_telemetry;
}

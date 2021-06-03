#include "payload.h"

// ZBTxStatusResponse txStatus = ZBTxStatusResponse();

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
  _xbee.onZBRxResponse(telemetry_handler_wrapper);
  _isPayloadDetected = false;
  _telemetry_from_xbee = "";
  _isPayloadTransmitting = false;
  txStatus = ZBTxStatusResponse();
}

bool Payload::checkForSuccess()
{
  bool sucess_trans = false;
  if (_xbee.readPacket(0.800))
  {
    // got a response!
    // should be a znet tx status
    if (_xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      _xbee.getResponse().getZBTxStatusResponse(txStatus);

      if (txStatus.getDeliveryStatus() == SUCCESS) {
        Serial.println("SUCCESS");
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
    Serial.println("xbee did not provide a TX status response")
  }
  return sucess_trans;
}


void Payload::connectToPayload()
{
  _isPayloadDetected = false;
  unsigned long last_time = millis();
  unsigned long miliseconds_limit = last_time + 100;

  request_message = "transmit_data"
  while(miliseconds_limit > last_time)
  {
    Serial.println("Connecting to payload");
    Serial.println("---------------------");

    Serial.println("Sending request from container");
    ZBTxRequest zbTx = ZBTxRequest(_addr64, request_message.c_str(), request_message.length());
    _xbee.send(zbTx);
    Serial.println("---------------------");

    //after sending the request, it is necessary
    //to check for a status response in order to
    //check the container and the xbee is not sleep
    _isPayloadDetected = checkForSuccess()
    if(_isPayloadDetected)
      break;
    last_time = millis();
  }
}

String Payload::send_cmd_and_receive_telemetry(String cmd)
{

  _telemetry_from_xbee = "No message";
  connectToPayload();
  if(_isPayloadDetected)
  {
    //send commands
    Serial.println("Sending data");
    ZBTxRequest zbTx = ZBTxRequest(_addr64, cmd.c_str(), cmd.length());
    _xbee.send(zbTx);
    if(checkForSuccess)
    {
      Serial.println("Command was sent from container to payload");
      if(cmd == "Payload On")
        _isPayloadTransmitting = true;
      else if(cmd == "Payload Off")
        _isPayloadTransmitting = false;
    }
    else
      Serial.println("Command was not sent, although payload was found");
    if(_isPayloadTransmitting)
    {
      //receive data from payload
      unsigned long last_time = millis();
      unsigned long miliseconds_limit = last_time + 100;
      while(_telemetry_from_xbee == "No message"
            && miliseconds_limit > last_time)
        last_time = millis();
    }
    return _telemetry_from_xbee;
  }
  else
  {
    Serial.println("Container did not find the xbee, skipping command")
  }
}


void GCS::telemetry_handler_wrapper(ZBRxResponse& rx, uintptr_t) {
  Serial.println("INSIDE telemetry_handler_wrapper");

    char buffer[rx.getDataLength() + 1];
    memcpy(buffer, rx.getFrameData() + rx.getDataOffset(), rx.getDataLength());
    buffer[rx.getDataLength()] = '\0';
    _telemetry_from_xbee = String(buffer);
}

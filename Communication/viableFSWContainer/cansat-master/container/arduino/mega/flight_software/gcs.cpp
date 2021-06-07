#include "gcs.h"
GCS::GCS(){}

GCS::GCS(String addr64, callback_function onAvailableCommandHandler, Stream &serial)
{
  // Take the last 8 characters as Low address
  String first_half = addr64.substring(addr64.length() - 8, addr64.length());
  // Take the rest as high address
  String second_half = addr64.substring(0, addr64.length() - 8);
  _addr64 = XBeeAddress64(strtoul(second_half.c_str(), NULL, 16),
                          strtoul(first_half.c_str(), NULL, 16));
  _serial = &serial;
  _onAvailableCommandHandler = onAvailableCommandHandler;
  _xbee.setSerial(*_serial);
  _xbee.onZBRxResponse(command_handler_wrapper);

  //
  // Serial.println("first_half");
  // Serial.println(first_half);
  // Serial.println(strtoul(first_half.c_str(), NULL, 16), HEX);
  // Serial.println("second_half");
  // Serial.println(second_half);
  // Serial.println(strtoul(second_half.c_str(), NULL, 16), HEX);
}
void GCS::insert_container_telemetry(String telemetry)
// void GCS::insert_container_telemetry(ContainerTelemetry telemetry)
{
  send_data(telemetry);
  // ZBTxRequest zbTx = ZBTxRequest(_addr64, telemetry.c_str(), telemetry.length());
  // _xbee.send(zbTx);

  // Check for success

}
// void GCS::insert_payload_telemetry(int payload, PayloadTelemetry telemetry)
void GCS::insert_payload_telemetry(int payload, String telemetry)
{
  send_data(telemetry);
}
void GCS::process_commands()
{
  // Serial.println("process_commands");
  _xbee.loop();
}

bool GCS::checkForSuccess()
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

void GCS::send_data(String data)
{
  Serial.println("Sending data: " + data);
  ZBTxRequest zbTx = ZBTxRequest(_addr64, data.c_str(), data.length());
  _xbee.send(zbTx);

  // TODO: Check for success
  checkForSuccess();
}

callback_function GCS::_onAvailableCommandHandler = NULL;

XBeeWithCallbacks GCS::_xbee;

void GCS::command_handler_wrapper(ZBRxResponse& rx, uintptr_t) {
  Serial.println("INSIDE command_handler_wrapper");

  if (_onAvailableCommandHandler)
  {
    char buffer[rx.getDataLength() + 1];
    memcpy(buffer, rx.getFrameData() + rx.getDataOffset(), rx.getDataLength());
    buffer[rx.getDataLength()] = '\0';
    String command = String(buffer);
    _onAvailableCommandHandler(command);
  }
}

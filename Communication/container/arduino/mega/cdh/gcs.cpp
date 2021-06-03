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

void GCS::send_data(String data)
{
  Serial.println("Sending data");
  ZBTxRequest zbTx = ZBTxRequest(_addr64, data.c_str(), data.length());
  _xbee.send(zbTx);

  // TODO: Check for success
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

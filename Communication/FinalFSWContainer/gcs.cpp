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
}
void GCS::insert_container_telemetry(String telemetry)
{
  send_data(telemetry);

}
void GCS::insert_payload_telemetry(String telemetry)
{
  send_data(telemetry);
}

String GCS::read_packet(unsigned int timeout)
{
  String packet = "";
  ZBRxResponse rxResponse = ZBRxResponse();

  if(_xbee.readPacket(timeout))
  {
    if (_xbee.getResponse().isAvailable()) {
      if (_xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        _xbee.getResponse().getZBRxResponse(rxResponse);

        char buffer[rxResponse.getDataLength() + 1];
        memcpy(buffer, rxResponse.getFrameData() + rxResponse.getDataOffset(), rxResponse.getDataLength());
        buffer[rxResponse.getDataLength()] = '\0';

        packet = String(buffer);

        unsigned long elapsed_time = millis();
      }
    }
  }
  return packet;
}

bool GCS::gcs_ack()
{
  String packet = read_packet(200);
  return packet == "ACK";
}


void GCS::process_commands()
{
  _done_handling_commands = false;

  // Make sure the command was sent and the gcs software acknowledged it received
  // the command
  if (send_data("GET COMMANDS") && gcs_ack())
  {
    while (!_done_handling_commands)
    {
      _xbee.loop();
    }
  }
  else
    Serial.println("Unable to communicate with GCS");
}

bool GCS::check_for_success()
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
        // Serial.println("SUCCESS");
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

bool GCS::send_data(String data)
{
  // Serial.println("Sending data: " + data);
  ZBTxRequest zbTx = ZBTxRequest(_addr64, (uint8_t*) data.c_str(), data.length());
  _xbee.send(zbTx);

  return check_for_success();
}

callback_function GCS::_onAvailableCommandHandler = NULL;

XBeeWithCallbacks GCS::_xbee;

bool GCS::_done_handling_commands = true;


void GCS::command_handler_wrapper(ZBRxResponse& rx, uintptr_t) {
  char buffer[rx.getDataLength() + 1];
  memcpy(buffer, rx.getFrameData() + rx.getDataOffset(), rx.getDataLength());
  buffer[rx.getDataLength()] = '\0';
  String command = String(buffer);

  // When finished command set flag
  if (command == "FINISHED")
    _done_handling_commands = true;
  else if (_onAvailableCommandHandler)
    _onAvailableCommandHandler(command);
}

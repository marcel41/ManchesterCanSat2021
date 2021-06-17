#include <XBee.h>
typedef void (*payload_callback_function)(); // type for conciseness

// class Telemetry
// {
//   private:
//     String telemetryData;
//   public:
//     static void setTelemetry(String);
//     static String getTelemetry();
//     Telemetry();
// };
// class Payload
// {
//   private:
//     static XBeeWithCallbacks _xbee;
//     XBeeAddress64 _addr64;
//     Stream* _serial;
//     // bool _isPayloadDetected;
//     // String _telemetry_from_xbee;
//     // Telemetry telemetry;
//     // bool _isPayloadTransmitting;
//     // ZBTxStatusResponse txStatus;
//     // ZBRxResponse rxResponse;
//     // XBeeResponse response;
//     // bool check_for_success();
//     // void connectToPayload();
//     static void telemetry_handler_wrapper(ZBRxResponse& rx, uintptr_t);
//   public:
//     String send_cmd_and_receive_telemetry(String);
//     Payload(String addr64, Stream &serial);
//     Payload();
// };


#include "telemetry.h"

class Payload
{
  private:
    static XBeeWithCallbacks _xbee;
    XBeeAddress64 _addr64;
    Stream* _serial;
    void send_data(String);
    static void on_rx_response(ZBRxResponse& rx, uintptr_t);
    bool check_for_success();
    static Payload* payloads[2];
    static int count;
    bool _connected;
    PayloadTelemetry _latest_telemetry;
    static String string_to_array_string(String data, char separator, int index);
  public:
    Payload(String addr64, Stream &serial);
    Payload();
    bool connected();
    static void update();
    PayloadTelemetry get_telemetry();
};

#include <XBee.h>
typedef void (*callback_function)(String); // type for conciseness

// class Telemetry
// {
//   private:
//     String telemetryData;
//   public:
//     static void setTelemetry(String);
//     static String getTelemetry();
//     Telemetry();
// };
class Payload
{
  private:
    static XBeeWithCallbacks _xbee;
    XBeeAddress64 _addr64;
    Stream* _serial;
    bool _isPayloadDetected;
    String _telemetry_from_xbee;
    // Telemetry telemetry;
    bool _isPayloadTransmitting;
    ZBTxStatusResponse txStatus;
    ZBRxResponse rxResponse;
    XBeeResponse response;
    bool checkForSuccess();
    void connectToPayload();
    static void telemetry_handler_wrapper(ZBRxResponse& rx, uintptr_t);
  public:
    String send_cmd_and_receive_telemetry(String);
    Payload(String addr64, Stream &serial);
    Payload();
};

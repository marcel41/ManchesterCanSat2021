#include <XBee.h>

typedef void (*callback_function)(String); // type for conciseness

class Payload
{
  private:
    static XBeeWithCallbacks _xbee;
    XBeeAddress64 _addr64;
    Stream* _serial;
    bool _isPayloadDetected;
    String _telemetry_from_xbee;
    bool _isPayloadTransmitting;
    ZBTxStatusResponse txStatus;
    bool checkForSuccess();
    void connectToPayload();
    void send_cmd_and_receive_telemetry(String);
    static void telemetry_handler_wrapper(ZBRxResponse& rx, uintptr_t);
  public:
    Payload(String addr64, Stream &serial);
    Payload();
};

#include <XBee.h>

typedef void (*callback_function)(String); // type for conciseness

class GCS
{
  private:
    static XBeeWithCallbacks _xbee;
    XBeeAddress64 _addr64;
    Stream* _serial;
    static callback_function _onAvailableCommandHandler;
    bool send_data(String);
    static void command_handler_wrapper(ZBRxResponse& rx, uintptr_t);
    bool check_for_success();
    bool gcs_ack();
    String read_packet(unsigned int);
    static bool _done_handling_commands;
  public:
    GCS(String addr64, callback_function onAvailableCommandHandler, Stream &serial);
    GCS();
    void insert_container_telemetry(String telemetry);
    // void insert_payload_telemetry(int payload, String telemetry);
    void insert_payload_telemetry(String telemetry);
    // void insert_payload_telemetry(int payload, PayloadTelemetry telemetry);
    void process_commands();
};

#include "esphome.h"

class SY210Sensor : public PollingComponent, public UARTDevice {
 public:
  Sensor *pm25_sensor = new Sensor();

  SY210Sensor(UARTComponent *parent) : PollingComponent(1000), UARTDevice(parent) {}

  void update() override {
    while (available() >= 5) {
      uint8_t buf[5];
      read_array(buf, 5);

      ESP_LOGD("sy210", "Raw frame: %02X %02X %02X %02X %02X",
               buf[0], buf[1], buf[2], buf[3], buf[4]);

      if (buf[0] == 0x42 && buf[1] == 0x4D) {
        uint16_t value = (buf[2] << 8) | buf[3];
        uint8_t checksum = (buf[0] + buf[1] + buf[2] + buf[3]) & 0xFF;

        if (checksum == buf[4]) {
          pm25_sensor->publish_state(value);
        } else {
          ESP_LOGW("sy210", "Checksum error: got %02X expect %02X",
                   buf[4], checksum);
        }
      } else {
        flush();
      }
    }
  }
};

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace sy210 {

class SY210Sensor : public PollingComponent, public uart::UARTDevice {
 public:
  sensor::Sensor *pm25_sensor;

  SY210Sensor(uart::UARTComponent *parent) : uart::UARTDevice(parent) {
    pm25_sensor = new sensor::Sensor();
  }

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
          this->pm25_sensor->publish_state(value);
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

}  // namespace sy210
}  // namespace esphome

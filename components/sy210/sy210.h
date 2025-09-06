#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include <vector>

namespace esphome {
namespace sy210 {

class SY210Sensor : public PollingComponent, public uart::UARTDevice {
 public:
  void set_pm25_sensor(sensor::Sensor *pm25_sensor) { pm25_sensor_ = pm25_sensor; }
  void set_pm10_sensor(sensor::Sensor *pm10_sensor) { pm10_sensor_ = pm10_sensor; }
  void set_pm1_0_sensor(sensor::Sensor *pm1_0_sensor) { pm1_0_sensor_ = pm1_0_sensor; }

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::DATA; }

 protected:
  sensor::Sensor *pm25_sensor_{nullptr};
  sensor::Sensor *pm10_sensor_{nullptr};
  sensor::Sensor *pm1_0_sensor_{nullptr};
  
  std::vector<uint8_t> buffer_;
  
  bool parse_frame(const uint8_t *data, size_t length);
  void process_data();
  void clear_buffer();
};

}  // namespace sy210
}  // namespace esphome
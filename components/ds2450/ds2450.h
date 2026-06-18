#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/one_wire/one_wire_bus.h"

namespace esphome {
namespace ds2450 {

class DS2450Sensor : public sensor::Sensor, public PollingComponent {
 public:
  void set_one_wire_bus(one_wire::OneWireBus *bus) { this->bus_ = bus; }
  void set_address(uint64_t address) { this->address_ = address; }
  void set_channel(uint8_t channel) { this->channel_ = channel; }
  void set_resolution(uint8_t resolution) { this->resolution_ = resolution; }

  void setup() override;
  void update() override;
  void dump_config() override;

 protected:
  one_wire::OneWireBus *bus_{nullptr};
  uint64_t address_{0};
  uint8_t channel_{0};
  uint8_t resolution_{8};

  bool configured_{false};

  bool configure_();
  bool start_conversion_();
  bool read_voltage_(float *voltage);
};

}  // namespace ds2450
}  // namespace esphome
#include "ds2450.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"

namespace esphome {
namespace ds2450 {

static const char *const TAG = "ds2450";

// DS2450 commands
static const uint8_t DS2450_CMD_WRITE_MEMORY = 0x55;
static const uint8_t DS2450_CMD_READ_MEMORY = 0xAA;
static const uint8_t DS2450_CMD_CONVERT = 0x3C;

// Addresses
static const uint8_t DS2450_ADDR_ADC_LO = 0x00;
static const uint8_t DS2450_ADDR_ADC_HI = 0x00;
static const uint8_t DS2450_ADDR_CTRL_LO = 0x08;
static const uint8_t DS2450_ADDR_CTRL_HI = 0x00;
static const uint8_t DS2450_ADDR_VCC_LO = 0x1C;
static const uint8_t DS2450_ADDR_VCC_HI = 0x00;

// Other
static const uint8_t DS2450_CONVERT_ALL_CHANNELS = 0x0F;
static const uint8_t DS2450_READOUT_CONTROL = 0xAA;
static const uint8_t DS2450_CONVERSION_COMPLETE = 0xFF;

// 5.12V range, no alarms, POR cleared
static const uint8_t DS2450_CONTROL_5V_RANGE = 0x01;

// VCC powered mode
static const uint8_t DS2450_VCC_POWERED = 0x40;

void DS2450Sensor::setup() {
  if (this->bus_ == nullptr) {
    ESP_LOGE(TAG, "No one_wire bus configured");
    this->mark_failed();
    return;
  }

  uint8_t family = this->address_ & 0xFF;
  if (family != 0x20) {
    ESP_LOGW(TAG, "Address family is 0x%02X, expected DS2450 family 0x20", family);
  }

  this->configured_ = this->configure_();
  if (!this->configured_) {
    ESP_LOGW(TAG, "Initial DS2450 configuration failed");
  }
}

void DS2450Sensor::dump_config() {
  char buffer[17];
  ESP_LOGCONFIG(TAG, "DS2450:");
  ESP_LOGCONFIG(TAG, "  Address: 0x%s", format_hex_to(buffer, this->address_));
  ESP_LOGCONFIG(TAG, "  Channel: %c", 'A' + this->channel_);
  ESP_LOGCONFIG(TAG, "  Resolution: %u bit", this->resolution_);
  LOG_UPDATE_INTERVAL(this);
}

void DS2450Sensor::update() {
  if (!this->configured_) {
    this->configured_ = this->configure_();
    if (!this->configured_) {
      ESP_LOGW(TAG, "DS2450 configuration failed");
      this->publish_state(NAN);
      return;
    }
  }

  if (!this->start_conversion_()) {
    ESP_LOGW(TAG, "DS2450 conversion start failed");
    this->publish_state(NAN);
    return;
  }

  float voltage = NAN;
  if (!this->read_voltage_(&voltage)) {
    ESP_LOGW(TAG, "DS2450 read failed");
    this->publish_state(NAN);
    return;
  }

  this->publish_state(voltage);
}

bool DS2450Sensor::configure_() {
  // Configure VCC powered mode.
  if (!this->bus_->select(this->address_))
    return false;

  this->bus_->write8(DS2450_CMD_WRITE_MEMORY);
  this->bus_->write8(DS2450_ADDR_VCC_LO);
  this->bus_->write8(DS2450_ADDR_VCC_HI);
  this->bus_->write8(DS2450_VCC_POWERED);

  // Read CRC bytes and verify byte, but for first version we don't enforce them.
  this->bus_->read8();
  this->bus_->read8();
  this->bus_->read8();

  // Configure all 4 channels: resolution + control byte.
  if (!this->bus_->select(this->address_))
    return false;

  this->bus_->write8(DS2450_CMD_WRITE_MEMORY);
  this->bus_->write8(DS2450_ADDR_CTRL_LO);
  this->bus_->write8(DS2450_ADDR_CTRL_HI);

  uint8_t resolution_byte = this->resolution_ == 16 ? 0x10 : 0x08;

  for (uint8_t i = 0; i < 4; i++) {
    this->bus_->write8(resolution_byte);
    this->bus_->read8();
    this->bus_->read8();
    this->bus_->read8();

    this->bus_->write8(DS2450_CONTROL_5V_RANGE);
    this->bus_->read8();
    this->bus_->read8();
    this->bus_->read8();
  }

  return true;
}

bool DS2450Sensor::start_conversion_() {
  if (!this->bus_->select(this->address_))
    return false;

  this->bus_->write8(DS2450_CMD_CONVERT);
  this->bus_->write8(DS2450_CONVERT_ALL_CHANNELS);
  this->bus_->write8(DS2450_READOUT_CONTROL);

  this->bus_->read8();
  this->bus_->read8();

  uint32_t start = millis();
  while (millis() - start < 100) {
    if (this->bus_->read8() == DS2450_CONVERSION_COMPLETE)
      return true;
    delay(1);
  }

  return false;
}

bool DS2450Sensor::read_voltage_(float *voltage) {
  if (!this->bus_->select(this->address_))
    return false;

  this->bus_->write8(DS2450_CMD_READ_MEMORY);
  this->bus_->write8(DS2450_ADDR_ADC_LO);
  this->bus_->write8(DS2450_ADDR_ADC_HI);

  uint8_t data[10];
  for (uint8_t i = 0; i < 10; i++) {
    data[i] = this->bus_->read8();
  }

  uint8_t value = data[1 + this->channel_ * 2];

  *voltage = float(value) * 5.10f / 255.0f;

  return true;
}

}  // namespace ds2450
}  // namespace esphome
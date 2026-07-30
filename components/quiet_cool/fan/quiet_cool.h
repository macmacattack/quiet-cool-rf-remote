#pragma once

#include "esphome/core/component.h"
#include "esphome/components/fan/fan.h"
#include "esphome/components/spi/spi.h"
#include "quietcool.h"

namespace esphome {
namespace quiet_cool {

class QuietCoolFan : public Component, public fan::Fan, public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW, spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_2MHZ> {
 public:
  QuietCoolFan() = default;

  void set_pins(uint8_t csn, uint8_t gdo0, uint8_t gdo2) {
    this->csn_pin_ = csn;
    this->gdo0_pin_ = gdo0;
    this->gdo2_pin_ = gdo2;
  }

  void set_remote_id(const std::vector<uint8_t> &remote_id) {
    for (size_t i = 0; i < 7 && i < remote_id.size(); i++) {
      this->remote_id_[i] = remote_id[i];
    }
  }

  void set_frequencies(float center_freq, float deviation) {
    this->center_freq_mhz_ = center_freq;
    this->deviation_khz_ = deviation;
  }

  void setup() override {
    // Instantiate QuietCool using ESP32-S3 Hardware SPI pins: SCK=12, MISO=13, MOSI=11
    this->qc_ = new QuietCool(this->csn_pin_, this->gdo0_pin_, this->gdo2_pin_, 12, 13, 11, this->remote_id_, this->center_freq_mhz_, this->deviation_khz_);
    this->qc_->begin();

    // Publish initial state to Home Assistant
    this->state = false;
    this->speed = 3;
    this->publish_state();
  }

  fan::FanTraits get_traits() override {
    auto traits = fan::FanTraits();
    traits.set_speed(true);
    traits.set_supported_speed_count(3);
    return traits;
  }

  void control(const fan::FanCall &call) override {
    if (call.get_state().has_value()) {
      bool new_state = *call.get_state();
      this->state = new_state;

      if (!new_state) {
        // Send OFF command
        this->qc_->send(QUIETCOOL_SPEED_HIGH, QUIETCOOL_DURATION_OFF);
      } else {
        // Send ON command using active speed
        int speed_val = this->speed;
        QuietCoolSpeed spd = QUIETCOOL_SPEED_HIGH;
        if (speed_val == 1) spd = QUIETCOOL_SPEED_LOW;
        else if (speed_val == 2) spd = QUIETCOOL_SPEED_MEDIUM;

        this->qc_->send(spd, QUIETCOOL_DURATION_ON);
      }
    } else if (call.get_speed().has_value()) {
      int speed_val = *call.get_speed();
      this->speed = speed_val;

      if (this->state) {
        QuietCoolSpeed spd = QUIETCOOL_SPEED_HIGH;
        if (speed_val == 1) spd = QUIETCOOL_SPEED_LOW;
        else if (speed_val == 2) spd = QUIETCOOL_SPEED_MEDIUM;

        this->qc_->send(spd, QUIETCOOL_DURATION_ON);
      }
    }
    this->publish_state();
  }

  void dump_config() override {
    ESP_LOGCONFIG("quiet_cool.fan", "QuietCool Fan:");
    ESP_LOGCONFIG("quiet_cool.fan", "  CS Pin: %d", this->csn_pin_);
    ESP_LOGCONFIG("quiet_cool.fan", "  GDO0 Pin: %d", this->gdo0_pin_);
    ESP_LOGCONFIG("quiet_cool.fan", "  GDO2 Pin: %d", this->gdo2_pin_);
  }

 protected:
  uint8_t csn_pin_{10};
  uint8_t gdo0_pin_{14};
  uint8_t gdo2_pin_{15};
  uint8_t remote_id_[7]{0};
  float center_freq_mhz_{433.897f};
  float deviation_khz_{10.0f};
  QuietCool *qc_{nullptr};
};

}  // namespace quiet_cool
}  // namespace esphome

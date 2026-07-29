#include "quiet_cool.h"
#include "esphome/core/log.h"

namespace esphome {
namespace quiet_cool {

static const char *const TAG = "quiet_cool.fan";

void QuietCoolFan::set_pins(uint8_t csn, uint8_t gdo0, uint8_t gdo2) {
  this->csn_pin_ = csn;
  this->gdo0_pin_ = gdo0;
  this->gdo2_pin_ = gdo2;
}

void QuietCoolFan::set_remote_id(const std::vector<uint8_t> &remote_id) {
  for (size_t i = 0; i < 7 && i < remote_id.size(); i++) {
    this->remote_id_[i] = remote_id[i];
  }
}

void QuietCoolFan::set_frequencies(float center_freq, float deviation) {
  this->center_freq_mhz_ = center_freq;
  this->deviation_khz_ = deviation;
}

void QuietCoolFan::setup() {
  // Pass ESP32-S3 Hardware SPI pins: SCK=12, MISO=13, MOSI=11
  this->qc_ = new QuietCool(this->csn_pin_, this->gdo0_pin_, this->gdo2_pin_, 12, 13, 11, this->remote_id_, this->center_freq_mhz_, this->deviation_khz_);
  this->qc_->begin();
}

fan::FanTraits QuietCoolFan::get_traits() {
  auto traits = fan::FanTraits();
  traits.set_speed(true);
  return traits;
}

void QuietCoolFan::control(const fan::FanCall &call) {
  if (call.get_state().has_value()) {
    bool state = *call.get_state();
    if (!state) {
      this->qc_->send(QUIETCOOL_SPEED_HIGH, QUIETCOOL_DURATION_OFF);
      this->state = false;
    } else {
      this->state = true;
    }
  }

  if (call.get_speed().has_value()) {
    int speed = *call.get_speed();
    if (this->state) {
      if (speed == 1) {
        this->qc_->send(QUIETCOOL_SPEED_LOW, QUIETCOOL_DURATION_ON);
      } else if (speed == 2) {
        this->qc_->send(QUIETCOOL_SPEED_MEDIUM, QUIETCOOL_DURATION_ON);
      } else {
        this->qc_->send(QUIETCOOL_SPEED_HIGH, QUIETCOOL_DURATION_ON);
      }
    }
    this->speed = speed;
  }
  this->publish_state();
}

void QuietCoolFan::dump_config() {
  ESP_LOGCONFIG(TAG, "QuietCool Fan:");
  ESP_LOGCONFIG(TAG, "  CS Pin: %d", this->csn_pin_);
  ESP_LOGCONFIG(TAG, "  GDO0 Pin: %d", this->gdo0_pin_);
  ESP_LOGCONFIG(TAG, "  GDO2 Pin: %d", this->gdo2_pin_);
}

}  // namespace quiet_cool
}  // namespace esphome

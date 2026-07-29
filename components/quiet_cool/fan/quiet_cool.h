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

  void set_pins(uint8_t csn, uint8_t gdo0, uint8_t gdo2);
  void set_remote_id(const std::vector<uint8_t> &remote_id);
  void set_frequencies(float center_freq, float deviation);

  void setup() override;
  fan::FanTraits get_traits() override;
  void control(const fan::FanCall &call) override;
  void dump_config() override;

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

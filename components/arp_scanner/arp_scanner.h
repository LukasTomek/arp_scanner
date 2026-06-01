#pragma once

#include "esphome/core/component.h"
#include <string>

namespace arp_scanner {

class ArpScannerComponent : public esphome::Component {
 public:
  void setup() override;
  void dump_config() override;

  // Triggers the background ping sweep
  void perform_scan(const std::string &base_ip);
};

}  // namespace arp_scanner

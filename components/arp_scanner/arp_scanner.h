#pragma once

#include "esphome/core/component.h"
#include <string>

namespace arp_scanner {

// Inherit explicitly from esphome::Component
class ArpScannerComponent : public esphome::Component {
 public:
  void setup() override;
  void dump_config() override;

  // Custom execution method called from YAML
  void perform_scan(const std::string &base_ip);
};

}  // namespace arp_scanner

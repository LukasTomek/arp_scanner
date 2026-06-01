#pragma once
#include "esphome.h"
#include <string>

namespace arp_scanner {

class ArpScannerComponent : public Component {
 public:
  // Lifecycle methods
  void setup() override;
  void dump_config() override;

  // Custom execution method called from YAML
  void perform_scan(const std::string &base_ip);
};

}  // namespace arp_scanner

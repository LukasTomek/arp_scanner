#include "arp_scanner.h"
#include "esphome/core/log.h"
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <lwip/ip_addr.h>

namespace arp_scanner {

static const char *const TAG = "arp_scan";

void ArpScannerComponent::setup() {
  ESP_LOGI(TAG, "ARP Scanner Component Initialized.");
}

void ArpScannerComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ARP Scanner Component Active.");
}

void ArpScannerComponent::perform_scan(const std::string &base_ip) {
  ESP_LOGI(TAG, "Starting network ARP scan for subnet: %s0/24", base_ip.c_str());

  struct netif *net_interface = netif_list;
  if (net_interface == nullptr) {
    ESP_LOGE(TAG, "No active network interface found!");
    return;
  }

  // Fire off explicit sequential ARP query broadcasts
  for (int i = 1; i < 255; i++) {
    std::string target_str = base_ip + std::to_string(i);
    ip4_addr_t target_ip;
    
    if (ip4addr_aton(target_str.c_str(), &target_ip)) {
      etharp_query(net_interface, &target_ip, NULL);
    }
    
    // Yield to the ESPHome framework loop to prevent watchdog reset
    delay(5); 
  }

  // Give network devices 250 milliseconds to stream responses back
  delay(250);

  ESP_LOGI(TAG, "--- Active Devices Found ---");
  
  // Parse through the local lwIP routing table cache
  for (int i = 0; i < ARP_TABLE_SIZE; i++) {
    ip4_addr_t *ip_entry;
    struct netif *netif_entry;
    struct eth_addr *mac_entry;

    if (etharp_get_entry(i, &ip_entry, &netif_entry, &mac_entry) == 1) {
      if (ip_entry->addr != 0) {
        // FIX: Allocate a proper 16-byte character array buffer for IPv4 string formatting
        char ip_buffer[16];
        ip4addr_ntoa_r(ip_entry, ip_buffer, sizeof(ip_buffer));
        
        // FIX: Explicitly target the 6 individual byte indices of the MAC array
        ESP_LOGI(TAG, "IP: %s | MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                 ip_buffer,
                 mac_entry->addr[0], mac_entry->addr[1], mac_entry->addr[2],
                 mac_entry->addr[3], mac_entry->addr[4], mac_entry->addr[5]);
      }
    }
  }
  ESP_LOGI(TAG, "----------------------------");
}

}  // namespace arp_scanner

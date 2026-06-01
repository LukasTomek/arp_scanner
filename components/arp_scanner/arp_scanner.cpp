#include "arp_scanner.h"
#include "esphome/core/log.h"
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <lwip/ip_addr.h>

// Include the standard Arduino Pico ping architecture
#include <Pinger.h>

namespace arp_scanner {

static const char *const TAG = "arp_scan";
static Pinger pinger;

void ArpScannerComponent::setup() {
  ESP_LOGI(TAG, "ARP/Ping Scanner Component Initialized.");
}

void ArpScannerComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ARP/Ping Scanner Component Active.");
}

void ArpScannerComponent::perform_scan(const std::string &base_ip) {
  ESP_LOGI(TAG, "Starting network sweep for subnet: %s0/24", base_ip.c_str());

  struct netif *net_interface = netif_list;
  if (net_interface == nullptr) {
    ESP_LOGE(TAG, "No active network interface found!");
    return;
  }

  // Ping every IP in the subnet to dynamically populate the ARP table cache
  for (int i = 1; i < 255; i++) {
    std::string target_str = base_ip + std::to_string(i);
    
    // Send a standard, single ICMP ping request. 
    // Timeout is set very low (20ms) because we don't care about waiting for the response here;
    // we just want to force a packet out so the target replies and populates the ARP cache.
    pinger.Ping(target_str.c_str(), 1, 20);
    
    // Crucial: yield to ESPHome to keep Wi-Fi connection stable
    delay(10); 
  }

  // Give lagging devices an extra moment to reply over the air
  delay(500);

  ESP_LOGI(TAG, "--- Active Devices Found in ARP Table ---");
  
  // Safely parse the populated lwIP table mapping
  for (int i = 0; i < ARP_TABLE_SIZE; i++) {
    ip4_addr_t *ip_entry;
    struct netif *netif_entry;
    struct eth_addr *mac_entry;

    if (etharp_get_entry(i, &ip_entry, &netif_entry, &mac_entry) == 1) {
      if (ip_entry->addr != 0) {
        char ip_buffer[16];
        ip4addr_ntoa_r(ip_entry, ip_buffer, sizeof(ip_buffer));
        
        ESP_LOGI(TAG, "IP: %s | MAC: %02X:%02X:%02X:%02X:%02X:%02X",
                 ip_buffer,
                 mac_entry->addr[0], mac_entry->addr[1], mac_entry->addr[2],
                 mac_entry->addr[3], mac_entry->addr[4], mac_entry->addr[5]);
      }
    }
  }
  ESP_LOGI(TAG, "-----------------------------------------");
}

}  // namespace arp_scanner

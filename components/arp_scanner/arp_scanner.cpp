#include "arp_scanner.h"
#include "esphome/core/log.h"
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <lwip/ip_addr.h>
#include <lwip/raw.h>
#include <lwip/icmp.h>

namespace arp_scanner {

static const char *const TAG = "arp_scan";

// Minimal structure representation of a standard basic 8-byte ping packet
struct icmp_echo_packet {
  uint8_t type;
  uint8_t code;
  uint16_t chksum;
  uint16_t id;
  uint16_t seq;
};

// Standard Internet Checksum computation algorithm
static uint16_t calculate_checksum(void *data, int length) {
  uint16_t *word_ptr = (uint16_t *)data;
  uint32_t total_sum = 0;
  for (; length > 1; length -= 2) {
    total_sum += *word_ptr++;
  }
  if (length == 1) {
    total_sum += *(uint8_t *)word_ptr;
  }
  while (total_sum >> 16) {
    total_sum = (total_sum & 0xFFFF) + (total_sum >> 16);
  }
  return (uint16_t)(~total_sum);
}

void ArpScannerComponent::setup() {
  ESP_LOGI(TAG, "ARP/ICMP Native Network Scanner Initialized.");
}

void ArpScannerComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ARP/ICMP Native Network Scanner Active.");
}

void ArpScannerComponent::perform_scan(const std::string &base_ip) {
  ESP_LOGI(TAG, "Starting network sweep for subnet: %s0/24", base_ip.c_str());

  struct netif *net_interface = netif_list;
  if (net_interface == nullptr) {
    ESP_LOGE(TAG, "No active network interface found!");
    return;
  }

  // Open a raw socket slot explicitly listening for the ICMP network protocol
  struct raw_pcb *icmp_socket = raw_new(IP_PROTO_ICMP);
  if (icmp_socket == nullptr) {
    ESP_LOGE(TAG, "Failed to allocate local raw ICMP socket stack context.");
    return;
  }

  // Prepare standard network echo frame configuration parameters
  struct icmp_echo_packet echo_packet;
  echo_packet.type = ICMP_ECHO; // Standard Echo request token ID
  echo_packet.code = 0;
  echo_packet.id = 0xBEEF;      // Arbitrary signature footprint ID
  echo_packet.seq = 1;
  echo_packet.chksum = 0;

  // Complete header checksum evaluation routine
  echo_packet.chksum = calculate_checksum(&echo_packet, sizeof(echo_packet));

  // Sequentially sweep every host address position block (.1 through .254)
  for (int i = 1; i < 255; i++) {
    std::string target_str = base_ip + std::to_string(i);
    ip_addr_t target_ip;
    
    if (ipaddr_aton(target_str.c_str(), &target_ip)) {
      // Allocate transient buffer container representation matching our core data structure length
      struct pbuf *packet_buffer = pbuf_alloc(PBUF_IP, sizeof(echo_packet), PBUF_RAM);
      if (packet_buffer != nullptr) {
        // Copy packet bytes into the allocated network buffer space
        pbuf_take(packet_buffer, &echo_packet, sizeof(echo_packet));
        
        // Push raw Layer 3 network segment frame payload down the wire directly
        raw_sendto(icmp_socket, packet_buffer, &target_ip);
        ESP_LOGI(TAG, "Send icmp packet to: %s", ipaddr_ntoa(&target_ip));
        // Free buffer to avoid a critical memory leak
        pbuf_free(packet_buffer);
      }
    }
    
    // Yield execution window context back to the central ESPHome watchdog process
    delay(5); 
  }

  // Allow 400 milliseconds for network targets to echo standard packet streams back
  delay(400);

  // Close and clean up the temporary native raw socket asset
  raw_remove(icmp_socket);
  ESP_LOGI(TAG, "--- Active Network Devices Discovered ---");
  
  // Safely index entries out from the standard lwIP routing tables directly
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

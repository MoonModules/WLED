#include "wled.h"
#include "Network.h"
#include "esp_netif.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

IPAddress NetworkClass::localIP() {
  esp_netif_ip_info_t ip_info;
  esp_netif_t* wifi_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  esp_netif_t* eth_netif = esp_netif_get_handle_from_ifkey("ETH_DEF");

  uint32_t wifi_metric = 999, eth_metric = 999;
  if (wifi_netif) wifi_metric = esp_netif_get_route_prio(wifi_netif);
  if (eth_netif) eth_metric = esp_netif_get_route_prio(eth_netif);

  // Check the preferred interface first (Ethernet by default)
  if (eth_metric < wifi_metric) {
    if (eth_netif && esp_netif_get_ip_info(eth_netif, &ip_info) == ESP_OK && ip_info.ip.addr != 0) {
      return IPAddress(ip_info.ip.addr); // Return Ethernet IP if valid
    }
  }
  if (wifi_netif && esp_netif_get_ip_info(wifi_netif, &ip_info) == ESP_OK && ip_info.ip.addr != 0) {
    return IPAddress(ip_info.ip.addr); // Return Wi-Fi IP if valid
  }
  return INADDR_NONE;
}

IPAddress NetworkClass::subnetMask() {
  esp_netif_ip_info_t ip_info;
  esp_netif_t* netif = esp_netif_get_default_netif();
  if (netif) {
    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
      return IPAddress(ip_info.netmask.addr);
    }
  }
  return IPAddress(0, 0, 0, 0);
}

IPAddress NetworkClass::gatewayIP() {
  esp_netif_ip_info_t ip_info;
  esp_netif_t* netif = esp_netif_get_default_netif();
  if (netif) {
    if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
      return IPAddress(ip_info.gw.addr);
    }
  }
  return INADDR_NONE;
}

void NetworkClass::localMAC(uint8_t* MAC) {
  memset(MAC, 0, 6);
  esp_netif_t* default_netif = esp_netif_get_default_netif();
  if (default_netif == NULL) {
    return; // No default interface is active
  }
  esp_netif_t* wifi_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  esp_netif_t* eth_netif = esp_netif_get_handle_from_ifkey("ETH_DEF");
  if (default_netif == wifi_netif) {
    esp_wifi_get_mac(WIFI_IF_STA, MAC);
  } else {
    esp_netif_get_mac(eth_netif, MAC);
  }
  return;
}

// bool NetworkClass::isConnected() {
//   esp_netif_t* netif = esp_netif_get_default_netif();
//   if (netif == NULL) {
//     return false;
//   }
//   esp_netif_ip_info_t ip_info;
//   if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
//     return (ip_info.ip.addr != 0);
//   }
//   return false;
// }

bool NetworkClass::isConnected() {
  esp_netif_t* netif = esp_netif_get_default_netif();
  if (netif == NULL) {
    return false;
  }
  esp_netif_ip_info_t ip_info;
  if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
    return (ip_info.ip.addr != 0) && (ip_info.ip.addr != IPADDR_BROADCAST);
  }
  return false;
}

IPAddress NetworkClass::hostByName(const char* hostname) {
  // Note: Arduino IPAddress is IPv4 only. This function will return
  // INADDR_NONE if the hostname resolves only to an IPv6 address.
  // Use getaddrinfo to perform the DNS lookup for ANY address family
  struct addrinfo hints = {
      .ai_family = AF_UNSPEC, // Allow either IPv4 or IPv6
      .ai_socktype = SOCK_STREAM,
  };
  struct addrinfo* res;
  if (getaddrinfo(hostname, NULL, &hints, &res) == 0 && res != NULL) {
    IPAddress result = INADDR_NONE;
    // Check the address family of the first result
    if (res->ai_family == AF_INET) {
      // It's an IPv4 address, which IPAddress can handle.
      struct in_addr* addr = &((struct sockaddr_in*)res->ai_addr)->sin_addr;
      result = IPAddress(addr->s_addr);
    } else if (res->ai_family == AF_INET6) {
      // It's an IPv6 address. The Arduino IPAddress object cannot store it.
      // You could log this if needed.
      ESP_LOGE("Network", "Hostname '%s' resolved to an IPv6 address, which is not supported.", hostname);
    }
    freeaddrinfo(res);
    return result;
  }
  // If we get here, the lookup failed
  return INADDR_NONE;
}

String NetworkClass::format_mac_address(const uint8_t* mac) {
  char mac_str[18];
  snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(mac_str);
}

esp_err_t NetworkClass::get_hardware_mac_address(uint8_t* mac_addr) {
  // This gets the MAC from the hardware, before any network service init happens.
  esp_err_t err = ESP_FAIL;
  #if defined(WLED_USE_ETHERNET) 
  if (eth_handle != NULL) {
    // Investigate esp_efuse_mac_get_default() in case we don't even need the eth_handle.
    // Just need to check with one this returns, assuming Ethernet on the P4.
    err = esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
  }
  if (err == ESP_OK) {
    return ESP_OK;
  }
  #elif !defined(WLED_USE_ETHERNET_ONLY)
  err = esp_wifi_get_mac(WIFI_IF_STA, mac_addr);
  #endif
  return err;
}

String NetworkClass::getEscapedMac() {
  uint8_t mac_addr[6];
  this->get_hardware_mac_address(mac_addr);
  String formatted_mac = this->format_mac_address(mac_addr);
  formatted_mac.replace(":", "");
  formatted_mac.toLowerCase();
  return formatted_mac;
}

bool NetworkClass::isEthernet() {
  return eth_is_connected;
}

bool NetworkClass::setHostname(const char* hostname) {
  esp_netif_t* netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
  if (netif == NULL) {
    ESP_LOGE("NetworkClass", "Failed to get STA network interface handle.");
    return false;
  }
  esp_err_t err = esp_netif_set_hostname(netif, hostname);
  if (err != ESP_OK) {
    ESP_LOGE("NetworkClass", "Failed to set hostname, error: %s", esp_err_to_name(err));
    return false;
  }
  ESP_LOGI("NetworkClass", "Hostname set to '%s'", hostname);
  return true;
}

#ifdef ARDUINO_ARCH_ESP32
#if defined(ESP_IDF_VERSION) && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
NetworkClass WL_Network;
#else
NetworkClass Network;
#endif
#else
NetworkClass Network;
#endif
#include "Network.h"

#ifdef WLED_QEMU
#include "esp_system.h"
#include "tcpip_adapter.h"
#endif

IPAddress NetworkClass::localIP()
{
#ifdef WLED_QEMU
  // QEMU: Get IP directly from tcpip_adapter
  tcpip_adapter_ip_info_t ip_info;
  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip_info) == ESP_OK) {
    if (ip_info.ip.addr != 0) {
      return IPAddress(ip_info.ip.addr);
    }
  }
  return INADDR_NONE;
#else
  IPAddress localIP;
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
  localIP = ETH.localIP();
  if (localIP[0] != 0) {
    return localIP;
  }
#endif
  localIP = WiFi.localIP();
  if (localIP[0] != 0) {
    return localIP;
  }

  return INADDR_NONE;
#endif
}

IPAddress NetworkClass::subnetMask()
{
#ifdef WLED_QEMU
  tcpip_adapter_ip_info_t ip_info;
  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip_info) == ESP_OK) {
    if (ip_info.netmask.addr != 0) {
      return IPAddress(ip_info.netmask.addr);
    }
  }
  return IPAddress(255, 255, 255, 0);
#else
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
  if (ETH.localIP()[0] != 0) {
    return ETH.subnetMask();
  }
#endif
  if (WiFi.localIP()[0] != 0) {
    return WiFi.subnetMask();
  }
  return IPAddress(255, 255, 255, 0);
#endif
}

IPAddress NetworkClass::gatewayIP()
{
#ifdef WLED_QEMU
  tcpip_adapter_ip_info_t ip_info;
  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip_info) == ESP_OK) {
    if (ip_info.gw.addr != 0) {
      return IPAddress(ip_info.gw.addr);
    }
  }
  return INADDR_NONE;
#else
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
  if (ETH.localIP()[0] != 0) {
      return ETH.gatewayIP();
  }
#endif
  if (WiFi.localIP()[0] != 0) {
      return WiFi.gatewayIP();
  }
  return INADDR_NONE;
#endif
}

void NetworkClass::localMAC(uint8_t* MAC)
{
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
  // ETH.macAddress(MAC); // Does not work because of missing ETHClass:: in ETH.ccp

  // Start work around
  String macString = ETH.macAddress();
  char macChar[18];
  char * octetEnd = macChar;

  strlcpy(macChar, macString.c_str(), 18);

  for (uint8_t i = 0; i < 6; i++) {
    MAC[i] = (uint8_t)strtol(octetEnd, &octetEnd, 16);
    octetEnd++;
  }
  // End work around

  for (uint8_t i = 0; i < 6; i++) {
    if (MAC[i] != 0x00) {
      return;
    }
  }
#endif
  WiFi.macAddress(MAC);
  return;
}

bool NetworkClass::isConnected()
{
#ifdef WLED_QEMU
  // QEMU: Check tcpip_adapter directly since ETH object is not initialized
  tcpip_adapter_ip_info_t ip_info;
  if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ip_info) == ESP_OK) {
    if (ip_info.ip.addr != 0) {
      return true;  // We have an IP from QEMU networking
    }
  }
  return false;
#else
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
  return (WiFi.localIP()[0] != 0 && WiFi.status() == WL_CONNECTED) || ETH.localIP()[0] != 0;
#else
  return (WiFi.localIP()[0] != 0 && WiFi.status() == WL_CONNECTED);
#endif
#endif
}

bool NetworkClass::isEthernet()
{
#ifdef WLED_QEMU
  return true;  // Always ethernet in QEMU mode
#else
#if defined(ARDUINO_ARCH_ESP32) && defined(WLED_USE_ETHERNET)
  return (ETH.localIP()[0] != 0);
#endif
  return false;
#endif
}

NetworkClass Network;
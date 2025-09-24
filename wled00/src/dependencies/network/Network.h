#ifdef ESP8266
  #include <ESP8266WiFi.h>
#elif defined (CONFIG_IDF_TARGET_ESP32S3) && defined (WLED_USE_ETHERNET)
  #include <WiFi.h>
  #include <ETHClass2.h>
  #define ETH ETH2
#else // ESP32
  #include <WiFi.h>
  #include <ETH.h>
#endif

#ifndef Network_h
#define Network_h

class NetworkClass
{
public:
  IPAddress localIP();
  IPAddress subnetMask();
  IPAddress gatewayIP();
  void localMAC(uint8_t* MAC);
  bool isConnected();
  bool isEthernet();
};

extern NetworkClass Network;

#endif
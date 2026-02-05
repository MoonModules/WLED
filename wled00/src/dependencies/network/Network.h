#ifdef ESP8266
  #include <ESP8266WiFi.h>
#else // ESP32
  #include <WiFi.h>
  #if defined(CONFIG_ETH_SPI_ETHERNET_W5500)
    #include "ETHClass2.h"
    extern ETHClass2 ETH;
  #else
   #include <ETH.h>
  #endif
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
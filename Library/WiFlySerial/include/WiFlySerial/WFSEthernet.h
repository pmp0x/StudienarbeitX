/*
 * WFSEthernet.h
 * Arduino Ethernet class for wifi devices
 * Based on Arduino 1.0 Ethernet class
 * 
 * Credits:
 * First to the Arduino Ethernet team for their model upon which this is based.
 * Modifications are
 * Copyright GPL 2.1 Tom Waldock 2012
 Version 1.07
 
 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.
 
 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.
 
 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 
 */
#ifndef WFSethernet_h
#define WFSethernet_h


#include <WiFlySerial/WiFlySerial.h>

#include "WiFlySerial/WFSIPAddress.h"
#include "WiFlySerial/WFSEthernetClient.h"
#include "WiFlySerial/WFSEthernetServer.h"


#define MAX_SOCK_NUM 1
#define WIFI_WAITING_TIME 500

class WFSEthernet : public Print {

public:
    static uint8_t _state[MAX_SOCK_NUM];
    static uint16_t _server_port[MAX_SOCK_NUM];
    
    WFSEthernet();
    void begin(WiFlySerial * wifi);
    
    // Devices often appreciate a chance to initialize before commencing operations
    //bool initDevice();
    
    // set up wifi association settings
    bool configure(uint8_t AuthMode, uint8_t JoinMode, uint8_t DCHPMode);
    bool credentials( char* pSSID, char* pPassphrase);
    bool setNTPServer( char* pNTPServer , float fTimeZoneOffsetHrs);
    
//    int begin();
//    int begin( WFSIPAddress local_ip);
//    int begin( WFSIPAddress local_ip, WFSIPAddress dns_server);
//    int begin( WFSIPAddress local_ip, WFSIPAddress dns_server, WFSIPAddress gateway);
//    int begin( WFSIPAddress local_ip, WFSIPAddress dns_server, WFSIPAddress gateway, WFSIPAddress subnet);
    
    //Connections and communication
   

    long getDeviceStatus();
    
    bool serveConnection();
        bool isConnectionOpen();
    bool connect(uint8_t * addr, uint16_t port);
    bool disconnect();
    
    virtual void flush();
    virtual uint8_t read();
    virtual void write(uint8_t byte);
    virtual int available();
    
    
    bool SendCommand(char * cmd);
    
   
    
    //IP related Functions
    
    WFSIPAddress localIP();
    WFSIPAddress subnetMask();
    WFSIPAddress gatewayIP();
    WFSIPAddress dnsServerIP();
    WFSIPAddress ntpServerIP();
    
    
    
    friend class WFSEthernetClient;
    friend class WFSEthernetServer;
    
    
private:
    WFSIPAddress _dnsServerAddress;
    WiFlySerial * _wifi;
};

//exposes the class and WiFly to be used by Client and server;
//extern WFSEthernet wifi;
//extern WiFlySerial wifi;



#endif

#ifndef __WIFLYSERIAL_H__
#define __WIFLYSERIAL_H__

/*
Arduino WiFly Device Driver
Driver for Roving Network's WiFly GSX (c) (tm) b/g WiFi device
 using a simple Tx/Rx serial connection.
 4-wires needed: Power, Gnd, Rx, Tx

Provides moderately-generic WiFi device interface.
Compatible with Arduino 1.0
Version 1.08

- WiFlyGSX is a relatively intelligent peer.
- WiFlyGSX may have awoken in a valid configured state while Arduino asleep;
    initialization and configuration to be polite and obtain state
- WiFlyGSX hardware CTS/RTS not enabled yet
- Can listen on multiple ports, only one at a time.
- most settings assumed volatile; fetched from WiFly where reasonable.

Expected pattern of use:
begin
issue commands, such as set SSID, passphrase etc
exit command mode / enter data mode
listen for web activity
Open a TCP/UDP connection to a peer
send / receive data
close connection

SoftwareSerial is exposed as serial i/o

Credits:
  SoftwareSerial   Mikal Hart   http://arduiniana.org/
  Time             Michael Margolis http://www.arduino.cc/playground/uploads/Code/Time.zip
  WiFly            Roving Networks   www.rovingnetworks.com
  and to Massimo and the Arduino team.


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

Copyright GPL 2.0 Tom Waldock 2011
*/

#include <libmaple.h>
#include <string.h>
#include <stdint.h>
#include <WiFlySerial/Debug.h>
#include <WiFlyShield/SpiUart.h>


#define COMMAND_BUFFER_SIZE 64
#define SMALL_COMMAND_BUFFER_SIZE 20
#define RESPONSE_BUFFER_SIZE 80
#define INDICATOR_BUFFER_SIZE 16
#define IP_ADDR_WIDTH  18
#define UC_N_IP_BYTES  4      // number of bytes per IP Address (need test for IPv6)

// default ports as set in WiFly - if you change these, make certain you call setRemotePort/setLocalPort
#define WIFLY_DEFAULT_REMOTE_PORT 80
#define WIFLY_DEFAULT_LOCAL_PORT 2000
#define WIFLY_DEFAULT_BAUD_RATE 9600

#define COMMAND_MODE_GUARD_TIME     300 // in milliseconds.  Must be at least 250ms.
#define DEFAULT_WAIT_TIME           1000UL  // waiting time for a command
#define ATTN_WAIT_TIME              1000UL  // waiting time for a reponse after a $$$ attention signal
#define JOIN_WAIT_TIME              10000UL  // joining a network could take longer
#define SERVING_WAIT_TIME           300000UL   // 5 minutes default wait time for serving a connection in milliseconds
#define WIFLY_RESTART_WAIT_TIME     2000UL  // wait time after issuing reboot command.
#define WIFLY_TIME_SYNC_INTERVAL    100UL   // 15 minute interval between clock resyncs
#define WIFLY_NTP_SYNC_INTERVAL     "60"    // sync with NTP server every hour


#define COMMAND_MODE_ENTER_RETRY_ATTEMPTS 5
#define COMMAND_RETRY_ATTEMPTS 3

// WiFly Responses
#define PROMPT_NONE                 0x0
#define PROMPT_EXPECTED_TOKEN_FOUND 0x1
#define PROMPT_READY                0x2
#define PROMPT_CMD_MODE             0x4
#define PROMPT_AOK                  0x8
#define PROMPT_OTHER                0x10
#define PROMPT_CMD_ERR              0x20
#define PROMPT_TIMEOUT              0x40
#define PROMPT_OPEN                 0x80
#define PROMPT_CLOSE                0x100
#define PROMPT_OPEN_ALREADY         0x200

#define N_PROMPTS               9
#define WIFLY_MSG_EXPECTED      0
#define WIFLY_MSG_AOK           1
#define WIFLY_MSG_CMD           2
#define WIFLY_MSG_ERR           3
#define WIFLY_MSG_PROMPT        4
#define WIFLY_MSG_PROMPT2       5
#define WIFLY_MSG_CLOSE         6
#define WIFLY_MSG_OPEN          7
#define WIFLY_MSG_OPEN_ALREADY  8

// Auth Modes for Network Authentication
// See WiFly manual for details
#define WIFLY_AUTH_OPEN        0    // Open (default)
#define WIFLY_AUTH_WEP         1    // WEP-128
#define WIFLY_AUTH_WPA1        2    // WPA1
#define WIFLY_AUTH_WPA1_2      3    // Mixed-mode WPA1 and WPA2-PSK
#define WIFLY_AUTH_WPA2_PSK    4    // WPA2-PSK
#define WIFLY_AUTH_ADHOC       6    // Ad-hoc, join any Ad-hoc network

// Join modes for joining networks
// See WiFly manual for details
#define WIFLY_JOIN_MANUAL      0    // Manual, do not try to join automatically
#define WIFLY_JOIN_AUTO        1    // Try to join the access point that matches the stored SSID, passkey and channel. (Default)
#define WIFLY_JOIN_ANY         2    // Join ANY access point with security matching the stored authentication mode. Ignores stored SSID and searches for strongest signal.
#define WIFLY_JOIN_MAKE_ADHOC  4    // Create an Ad-Hoc network using stored SSID, IP and netmask.  Channel MUST be set. DHCP should be 0.

// DHCP modes
// See WiFly manual for details
#define WIFLY_DHCP_OFF         0    // DHCP OFF, use static IP address
#define WIFLY_DHCP_ON          1    // DHCP ON, get IP, gateway from Access Point.
#define WIFLY_DHCP_AUTO        2    // DHCP AUTO-IP, generally used with Adhoc networks
#define WIFLY_DHCP_CACHE       3    // DHCP CACHE, use previous IP address if lease is not expired.

// IP modes: UDP and TCP
#define WIFLY_IPMODE_UDP       0x02;    // Same as W5100_defn's SmMR::UDP
#define WIFLY_IPMODE_TCP       0x01;    // Same as W5100_defn's SmMR::TCP

#define WIFLY_STATUS_OFFLINE    0
#define WIFLY_STATUS_ONLINE     1



// WiFly-specific prompt codes
static char* WiFlyFixedPrompts[N_PROMPTS] = { "","AOK", "CMD", "ERR: ?", "",">","*CLOS*","*OPEN*","ERR:Connected" };
static byte  WiFlyFixedFlags[N_PROMPTS] = {PROMPT_EXPECTED_TOKEN_FOUND, PROMPT_AOK, PROMPT_CMD_MODE, PROMPT_CMD_ERR, PROMPT_READY,      PROMPT_READY,PROMPT_CLOSE, PROMPT_OPEN, PROMPT_OPEN_ALREADY};




// Strings stored in Program space
const char  s_WIFLYDEVICE_LIBRARY_VERSION[] __FLASH__ = "WiFlySerial v1.08" ;
const char  s_WIFLYDEVICE_JOIN[] __FLASH__ = "join " ;
const char  s_WIFLYDEVICE_OPEN[] __FLASH__ = "open " ;
const char  s_WIFLYDEVICE_CLOSE[] __FLASH__ = "close" ;
const char  s_WIFLYDEVICE_ASSOCIATED[] __FLASH__ = "ssociated" ;
const char  s_WIFLYDEVICE_ATTN[] __FLASH__ = "$$$";
const char  s_WIFLYDEVICE_VER[] __FLASH__ = "ver" ;
const char  s_WIFLYDEVICE_LEAVE_CMD_MODE[] __FLASH__ ="exit";
const char  s_WIFLYDEVICE_REBOOT[] __FLASH__ ="reboot";
const char  s_WIFLYDEVICE_SAVE[] __FLASH__ ="save";
const char  s_WIFLYDEVICE_GET_MAC[] __FLASH__ =" get mac";
const char  s_WIFLYDEVICE_GET_MAC_ADDR[] __FLASH__ ="Addr=";
const char  s_WIFLYDEVICE_GET_IP[] __FLASH__ =" get ip";
const char  s_WIFLYDEVICE_GET_GW[] __FLASH__ = " "; // "GW=";
const char  s_WIFLYDEVICE_GET_NM[] __FLASH__ = " "; // "NM=";
const char  s_WIFLYDEVICE_LEAVE[] __FLASH__ ="leave";
const char  s_WIFLYDEVICE_SET_SSID[] __FLASH__ =" set wlan s ";
const char  s_WIFLYDEVICE_SET_CHANNEL[] __FLASH__ =" set wlan c ";
const char  s_WIFLYDEVICE_SET_WIFI_AUTH[] __FLASH__ =" set wlan a ";
const char  s_WIFLYDEVICE_SET_WIFI_JOIN[] __FLASH__ =" set wlan j ";
const char  s_WIFLYDEVICE_SET_PASSPHRASE[] __FLASH__ =" set w p ";
const char  s_WIFLYDEVICE_NETWORK_SCAN[] __FLASH__ ="scan ";
const char  s_WIFLYDEVICE_AOK[] __FLASH__ ="";
const char  s_WIFLYDEVICE_SET_UART_BAUD[] __FLASH__ ="set u b 9600 ";
const char  s_WIFLYDEVICE_DEAUTH[] __FLASH__ ="Deauth";
const char  s_WIFLYDEVICE_SET_NTP[] __FLASH__ =" set time a ";
const char  s_WIFLYDEVICE_SET_NTP_ENABLE[] __FLASH__ ="set time e ";
const char  s_WIFLYDEVICE_SET_DEVICEID[] __FLASH__ ="set opt deviceid ";
const char  s_WIFLYDEVICE_IP_DETAILS[] __FLASH__ ="get ip";
const char  s_WIFLYDEVICE_GET_DNS_DETAILS[] __FLASH__ ="get dns";
const char  s_WIFLYDEVICE_GET_TIME[] __FLASH__ ="show t t";
const char  s_WIFLYDEVICE_SET_DHCP[] __FLASH__ ="set ip dhcp ";
const char  s_WIFLYDEVICE_SET_IP[] __FLASH__ ="set ip a ";
const char  s_WIFLYDEVICE_SET_NETMASK[] __FLASH__ ="set ip n ";
const char  s_WIFLYDEVICE_SET_GATEWAY[] __FLASH__ ="set ip g ";
const char  s_WIFLYDEVICE_SET_DNS[] __FLASH__ ="set dns addr ";
const char  s_WIFLYDEVICE_SET_LOCAL_PORT[] __FLASH__ ="set ip local ";
const char  s_WIFLYDEVICE_SET_REMOTE_PORT[] __FLASH__ ="set ip remote  ";
const char  s_WIFLYDEVICE_SET_PROTOCOL[] __FLASH__ ="set ip proto ";
const char  s_WIFLYDEVICE_ERR_REBOOOT[] __FLASH__ ="Attempting reboot";
const char  s_WIFLYDEVICE_ERR_START_FAIL[] __FLASH__ ="Failed to get cmd prompt:Halted.";
const char  s_WIFLYDEVICE_SET_UART_MODE[] __FLASH__ ="set u m 1 ";
const char  s_WIFLYDEVICE_GET_WLAN[] __FLASH__ ="get wlan ";
const char  s_WIFLYDEVICE_GET_RSSI[] __FLASH__ ="show rssi ";
const char  s_WIFLYDEVICE_GET_BATTERY[] __FLASH__ ="show batt ";
const char  s_WIFLYDEVICE_GET_STATUS[] __FLASH__ ="show conn ";
const char  s_WIFLYDEVICE_RETURN[] __FLASH__ ="\r";

const char  s_WIFLYDEVICE_GET_IP_IND[] __FLASH__ ="IP=";
const char  s_WIFLYDEVICE_GET_NM_IND[] __FLASH__ ="NM=";
const char  s_WIFLYDEVICE_GET_GW_IND[] __FLASH__ ="GW=";
const char  s_WIFLYDEVICE_GET_DNS_IND[] __FLASH__ ="DNS=";
const char  s_WIFLYDEVICE_GET_WLAN_SSID_IND[] __FLASH__ ="SSID=";
const char  s_WIFLYDEVICE_GET_RSSI_IND[] __FLASH__ ="RSSI=";
const char  s_WIFLYDEVICE_GET_WLAN_DEV_IND[] __FLASH__ ="DeviceID=";
const char  s_WIFLYDEVICE_GET_BATTERY_IND[] __FLASH__ ="Batt=";
const char  s_WIFLYDEVICE_GET_TIME_IND[] __FLASH__ ="RTC=";
const char  s_WIFLYDEVICE_GET_STATUS_IND[] __FLASH__ ="8";
const char  s_WIFLYDEVICE_GET_IP_UP_IND[] __FLASH__ ="F=";

// Index of strings
#define STI_WIFLYDEVICE_INDEX_JOIN        0
#define STI_WIFLYDEVICE_INDEX_ASSOCIATED  1
#define STI_WIFLYDEVICE_ATTN              2
#define STI_WIFLYDEVICE_VER               3
#define STI_WIFLYDEVICE_GET_MAC           4
#define STI_WIFLYDEVICE_GET_IP            5
#define STI_WIFLYDEVICE_GET_GW            6
#define STI_WIFLYDEVICE_GET_NM            7
#define STI_WIFLYDEVICE_LEAVE             8
#define STI_WIFLYDEVICE_SET_SSID          9
#define STI_WIFLYDEVICE_SET_PASSPHRASE    10
#define STI_WIFLYDEVICE_NETWORK_SCAN      11
#define STI_WIFLYDEVICE_AOK               12
#define STI_WIFLYDEVICE_SET_UART_BAUD     13
#define STI_WIFLYDEVICE_DEAUTH            14
#define STI_WIFLYDEVICE_SET_NTP           15
#define STI_WIFLYDEVICE_SET_NTP_ENABLE    16
#define STI_WIFLYDEVICE_SET_DEVICEID      17
#define STI_WIFLYDEVICE_GET_IP_DETAILS    18
#define STI_WIFLYDEVICE_LEAVE_CMD_MODE    19
#define STI_WIFLYDEVICE_GET_DNS_DETAILS   20
#define STI_WIFLYDEVICE_GET_TIME          21
#define STI_WIFLYDEVICE_SET_DHCP          22
#define STI_WIFLYDEVICE_SET_IP            23
#define STI_WIFLYDEVICE_SET_NETMASK       24
#define STI_WIFLYDEVICE_SET_GATEWAY       25
#define STI_WIFLYDEVICE_SET_DNS           26
#define STI_WIFLYDEVICE_ERR_REBOOT        27
#define STI_WIFLYDEVICE_ERR_START_FAIL    28
#define STI_WIFLYDEVICE_SET_UART_MODE     29
#define STI_WIFLYDEVICE_GET_WLAN          30
#define STI_WIFLYDEVICE_GET_RSSI          31
#define STI_WIFLYDEVICE_GET_BATTERY       32
#define STI_WIFLYDEVICE_LIBRARY_VERSION   33
#define STI_WIFLYDEVICE_SET_CHANNEL       34
#define STI_WIFLYDEVICE_SET_WIFI_AUTH     35
#define STI_WIFLYDEVICE_SET_WIFI_JOIN     36
#define STI_WIFLYDEVICE_GET_STATUS        37
#define STI_WIFLYDEVICE_GET_MAC_ADDR      38
#define STI_WIFLYDEVICE_RETURN            39
#define STI_WIFLYDEVICE_GET_IP_IND        40
#define STI_WIFLYDEVICE_GET_NM_IND        41
#define STI_WIFLYDEVICE_GET_GW_IND        42
#define STI_WIFLYDEVICE_GET_DNS_IND       43
#define STI_WIFLYDEVICE_GET_WLAN_SSID_IND 44
#define STI_WIFLYDEVICE_GET_RSSI_IND      45
#define STI_WIFLYDEVICE_GET_BATTERY_IND   46
#define STI_WIFLYDEVICE_GET_WLAN_DEV_IND  47
#define STI_WIFLYDEVICE_GET_TIME_IND      48
#define STI_WIFLYDEVICE_GET_STATUS_IND    49
#define STI_WIFLYDEVICE_GET_IP_UP_IND     50
#define STI_WIFLYDEVICE_OPEN              51
#define STI_WIFLYDEVICE_REBOOT            52
#define STI_WIFLYDEVICE_CLOSE             53
#define STI_WIFLYDEVICE_SET_LOCAL_PORT    54
#define STI_WIFLYDEVICE_SET_REMOTE_PORT   55
#define STI_WIFLYDEVICE_SET_PROTOCOL      56
#define STI_WIFLYDEVICE_SAVE              57

// String Table in Program space
const char * WiFlyDevice_string_table[] =
{
  // 0-based index, see STI_WIFLY_DEVICE_ list above.
  s_WIFLYDEVICE_JOIN,
  s_WIFLYDEVICE_ASSOCIATED,
  s_WIFLYDEVICE_ATTN,
  s_WIFLYDEVICE_VER,
  s_WIFLYDEVICE_GET_MAC,
  s_WIFLYDEVICE_GET_IP,
  s_WIFLYDEVICE_GET_GW,
  s_WIFLYDEVICE_GET_NM,
  s_WIFLYDEVICE_LEAVE,
  s_WIFLYDEVICE_SET_SSID,
  // 10 follows
  s_WIFLYDEVICE_SET_PASSPHRASE,
  s_WIFLYDEVICE_NETWORK_SCAN,
  s_WIFLYDEVICE_AOK,
  s_WIFLYDEVICE_SET_UART_BAUD,
  s_WIFLYDEVICE_DEAUTH,
  s_WIFLYDEVICE_SET_NTP,
  s_WIFLYDEVICE_SET_NTP_ENABLE,
  s_WIFLYDEVICE_SET_DEVICEID,
  s_WIFLYDEVICE_IP_DETAILS,
  s_WIFLYDEVICE_LEAVE_CMD_MODE,
  // 20 follows
  s_WIFLYDEVICE_GET_DNS_DETAILS,
  s_WIFLYDEVICE_GET_TIME,
  s_WIFLYDEVICE_SET_DHCP,
  s_WIFLYDEVICE_SET_IP,
  s_WIFLYDEVICE_SET_NETMASK,
  s_WIFLYDEVICE_SET_GATEWAY,
  s_WIFLYDEVICE_SET_DNS,
  s_WIFLYDEVICE_ERR_REBOOOT,
  s_WIFLYDEVICE_ERR_START_FAIL,
  s_WIFLYDEVICE_SET_UART_MODE,
  // 30 follows
  s_WIFLYDEVICE_GET_WLAN,
  s_WIFLYDEVICE_GET_RSSI,
  s_WIFLYDEVICE_GET_BATTERY,
  s_WIFLYDEVICE_LIBRARY_VERSION,
  s_WIFLYDEVICE_SET_CHANNEL,
  s_WIFLYDEVICE_SET_WIFI_AUTH,
  s_WIFLYDEVICE_SET_WIFI_JOIN,
  s_WIFLYDEVICE_GET_STATUS,
  s_WIFLYDEVICE_GET_MAC_ADDR,
  s_WIFLYDEVICE_RETURN,
  // 40 follows
  s_WIFLYDEVICE_GET_IP_IND,
  s_WIFLYDEVICE_GET_NM_IND,
  s_WIFLYDEVICE_GET_GW_IND,
  s_WIFLYDEVICE_GET_DNS_IND,
  s_WIFLYDEVICE_GET_WLAN_SSID_IND,
  s_WIFLYDEVICE_GET_RSSI_IND,
  s_WIFLYDEVICE_GET_BATTERY_IND,
  s_WIFLYDEVICE_GET_WLAN_DEV_IND,
  s_WIFLYDEVICE_GET_TIME_IND,
  s_WIFLYDEVICE_GET_STATUS_IND,
  // 50 follows
  s_WIFLYDEVICE_GET_IP_UP_IND,
  s_WIFLYDEVICE_OPEN,
  s_WIFLYDEVICE_REBOOT,
  s_WIFLYDEVICE_CLOSE,
  s_WIFLYDEVICE_SET_LOCAL_PORT,
  s_WIFLYDEVICE_SET_REMOTE_PORT,
  s_WIFLYDEVICE_SET_PROTOCOL,
  s_WIFLYDEVICE_SAVE
};

// Utility functions
//char* IP_ArrayToBuffer( const uint8_t* pIP, char* pBuffer, int buflen);
//uint8_t* BufferToIP_Array(char* pBuffer, uint8_t* pIP)  ;



class WiFlySerial : public Print {
  public:
    // Constructors
    WiFlySerial();

    // Destructor

    // Initialization
    bool begin(SpiUartDevice * TheSpi);  // Initialises this interface Class.

    // // Status
    //
    //  // Obtain current device status flags
    //  long    getDeviceStatus();  // refreshes device status flags.
    //  bool isAssociated();
    //  bool isTCPConnected();
    //  bool isAuthenticated();
    //  bool isifUp();
    //  int     getChannel();
    //  bool isDNSfound();
    //  bool isDNScontacted();
    //  bool isInCommandMode();
    //  bool isConnectionOpen();
    //
    //  // Information
    //  // Device Info
    //  char* showNetworkScan( char* pNetScan, const int buflen);
    //  char* getLibraryVersion(char* buf, int buflen);
    //
    //  // IP info
    //  char* getSSID(char* buf, int buflen);
    //  char* getDeviceID(char* buf, int buflen);
    //  char* getIP(char* buf, int buflen);
    //  char* getNetMask(char* buf, int buflen);
    //  char* getGateway(char* buf, int buflen);
    //  char* getDNS(char* buf, int buflen);
    //  char* getMAC(char* buf, int buflen);
    //  char* getNTP(char* buf, int buflen);
    //  char* getNTP_Update_Frequency(char* buf, int buflen);
    //  unsigned long getTime();
    //  char* getRSSI(char* pBuf, int buflen);
    //  char* getBattery(char* pBuf, int buflen);
    //
    //  // Transmit / Receive / available through exposed SoftwareSerial
    //  SoftwareSerial uart;
    //
    //  // Configuration Generic Wifi methods
    //
    //  // Generic Wifi methods
    //  bool setSSID( const char* pSSID);
    //  bool setPassphrase( const char* pPassphrase);
    //  bool setDeviceID( const char* pHostname);
    //  bool setNTP(const char* pNTP_IP);
    //  bool setNTP_Update_Frequency(const char* pFreq);
    //  bool setNTP_UTC_Offset(float fltUTC_Offset_hours);
    //  bool setIP( const char* pIP);
    //  bool setNetMask( const char* pNM);
    //  bool setGateway( const char* pGW);
    //  bool setDNS( const char* pDNS);
    //  bool setChannel( const char* pChannel);
    //  bool setLocalPort( int iNewLocalPort = WIFLY_DEFAULT_LOCAL_PORT);
    //  bool setRemotePort( int iNewRemotePort = WIFLY_DEFAULT_REMOTE_PORT);
    //  bool setProtocol( unsigned int iProtocol);
    //  bool setAuthMode( int iAuthMode);
    //  bool setJoinMode( int iJoinMode);
    //  bool setDHCPMode(const int iModeDHCP);
    //
    //  // wifi network Association
    //
    //  // Joins ssid set with setSSID
    //  bool join();
    //  bool join( char* pSSID);
    //
    //  // Leaves current SSID.
    //  bool leave();
    //
    //  // Generic utility
        bool StartCommandMode(char* pBuffer = NULL, const int bufSize = COMMAND_BUFFER_SIZE );
    //  bool exitCommandMode();
    //  void    reboot();
    //
    //  // Client Connection
    //  bool openConnection(const char* pURL, const unsigned long WaitTime = JOIN_WAIT_TIME  );
    //  bool closeConnection(bool bSafeClose = true);
    //
    //  // Server Connection - waits for a client to connect
    //  bool serveConnection(  const unsigned long reconnectWaitTIme = SERVING_WAIT_TIME );
    //
    //  // Open-format for RN 131C/G commands
    //  bool SendInquiry(char *Command, char* pBuffer, const int bufsize = RESPONSE_BUFFER_SIZE );
    //  bool SendInquirySimple(char *Command);
    //  bool SendCommand( char *Command,   char *SuccessIndicator, char* pBuffer, const int bufsize,
    //              const bool bCollecting = true, const unsigned long WaitTime = DEFAULT_WAIT_TIME ,
    //              const bool bClear = true, const bool bPromptAfterResult = true );
    //  bool SendCommandSimple( char *Command,   char *SuccessIndicator);
    //
    //  // utilities for collecting results or scanning for indicators.
     int     ScanForPattern( char* responseBuffer, const int bufsize, const char *pExpectedPrompt,
                  const bool bCapturing = true, const unsigned long WaitTime = DEFAULT_WAIT_TIME, const bool bPromptAfterResult = true   );
    //  char*   ExtractDetail(char* pCommand, char* pDetail, const int buflen, const char* pFrom, const char* pTo);
    //  char*   ExtractDetailIdx(const int idxCommand, char* pDetail, int buflen, const int idxSearch, const int idxStop);
    //  int     CaptureUntilPrompt( char* responseBuffer, const int bufsize, const char *pExpectedPrompt, const unsigned long WaitTime = DEFAULT_WAIT_TIME  );
    //
    //  int peek();
    //  virtual size_t write(uint8_t byte);
    //  virtual int read();
    //  virtual int available();
    //  virtual void flush();
    //
    //  int drain ();
    //
    //  using Print::write;
    //
    //  // debug utilities - use Serial : not NewSoftSerial as it will affect incoming stream.
    //  // should change these to use stream <<
    //  void    setDebugChannel( Print* pDebug);
    //  Print*  getDebugChannel( )  { return pDebugChannel; };
    //  void    clearDebugChannel();
    //  void    DebugPrint( const char* pMessage);
    //  void    DebugPrint( const int iNumber);
    //  void    DebugPrint( const char ch);

    private:
        SpiUartDevice * uart;
        // internal buffer for command-prompt
        char    szWiFlyPrompt[INDICATOR_BUFFER_SIZE ];

        // Internal status flags
        long    fStatus;
        bool bWiFlyInCommandMode;
        bool bWiFlyConnectionOpen;
        char*   pControl;

        // Ports for connections
        int     iRemotePort;
        int     iLocalPort;
        long    lUTC_Offset_seconds;


        // bool GetCmdPrompt();
        char*   GetBuffer_P(const int StringIndex, char* pBuffer, int bufSize);
        //      char*   ExtractLineFromBuffer(const int idString,  char* pBuffer, const int bufsize, const char* pStartPattern, const char* chTerminator);
        //      bool issueSetting( int idxCommand, const char* pParam);

        // Internal debug channel.
       // Print*  pDebugChannel;


};

// static  WiFlySerialv2 wifi ( ARDUINO_RX_PIN, ARDUINO_TX_PIN);

#endif

/*
 * WFSEthernet.cpp
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

#include <WiFlySerial/WiFlySerial.h>
#include "WiFlySerial/WFSEthernet.h"
#include <Time.h>
//#include "Dhcp.h"


WFSEthernet::WFSEthernet(){
    
    
}

void WFSEthernet::begin(WiFlySerial * wifi){
    _wifly = wifi;
    _activeClient = false;
}





//init
//Allows device to prepare for operations.
//
// Parameters: none
// Returns true on success, false on failure.
//bool WFSEthernet::initDevice() {
//  
//  return _wifly->begin();
//}

//configure
// Sets up modes of operation.
//
//Parameters
// AuthMode  - authentication modes
// JoinMode  - Means of joining a network
// DHCPMode  - Options for DHCP
//
// Returns true on success, false on failure.
bool WFSEthernet::configure(uint8_t  AuthMode, uint8_t JoinMode, uint8_t  DHCPMode) {
  
   return _wifly->setAuthMode(AuthMode) | _wifly->setJoinMode(JoinMode) | _wifly->setDHCPMode(DHCPMode) ;
  
}

// credentials
// Sets wifi name and passphrase/key
//
// Parameters
// pSSID      SSID to join
// pPassphrase WPA/2 Passphrase or WEP key
//
// Returns true on success, false on failure.
bool WFSEthernet::credentials( char* pSSID, char* pPassphrase) {
  
  bool bReturn =  ( _wifly->setPassphrase(pPassphrase) & _wifly->setSSID(pSSID) );
  return bReturn;
}

// begin()s
// call init(), then configure(), then credentials() 
/// BEFORE calling begin().
// fewer-parameter begins cascade to more-parameter methods.
// work really happens at last method.

//int WFSEthernet::begin()
//{
//  // Initialise the basic info
//  WFSIPAddress ip_addr(0,0,0,0);
//  
//  return begin(ip_addr);
//  
//}
//
//int WFSEthernet::begin( WFSIPAddress local_ip)
//{
//  // Assume the DNS server will be the machine on the same network as the local IP
//  // but with last octet being '1'
//  WFSIPAddress dns_server = local_ip;
//  dns_server[3] = 1;
//  return begin( local_ip, dns_server);
//}
//
//int WFSEthernet::begin( WFSIPAddress local_ip, WFSIPAddress dns_server)
//{
//  // Assume the gateway will be the machine on the same network as the local IP
//  // but with last octet being '1'
//  WFSIPAddress gateway = local_ip;
//  gateway[3] = 1;
//  return begin( local_ip, dns_server, gateway);
//}
//
//int WFSEthernet::begin( WFSIPAddress local_ip, WFSIPAddress dns_server, WFSIPAddress gateway)
//{
//  WFSIPAddress subnet(255, 255, 255, 0);
//  return begin( local_ip, dns_server, gateway, subnet);
//}
//
//int WFSEthernet::begin( WFSIPAddress local_ip, WFSIPAddress dns_server, WFSIPAddress gateway, WFSIPAddress subnet)
//{
//    //wifi.begin();
//  
//  // Arduino-WiFly communication-stability settings
//  char bufRequest[COMMAND_BUFFER_SIZE];
//  
//  // Set echo off
//  _wifly->SendCommand("set u m 0x1",">",bufRequest, COMMAND_BUFFER_SIZE);
//  
//  // Auto-disconnect after 30 seconds of idle connection.
//  _wifly->SendCommand( "set comm idle 30",">"  ,bufRequest, COMMAND_BUFFER_SIZE)  ;
//    
//  // Auto-sends TCP packet if no additional bytes provided from Arduino (ms).
//  _wifly->SendCommand( "set comm time 1000",">",bufRequest, COMMAND_BUFFER_SIZE);
//    
//  // Auto-sends TCP packet once this number of bytes provided from Arduino.
//  _wifly->SendCommand("set comm size 256",">",bufRequest, COMMAND_BUFFER_SIZE);
//  
//  // if already joined then leave first
//  _wifly->getDeviceStatus();
//  if (_wifly->isAssociated() ) {
//    DEBUG_LOG(3, "leaving");
//    _wifly->leave();
//  }
//  
//  // Try to join network based on provided (?) settings.
//  
//  int iRetry = 0;
//  bool bJoined = false;
//  while ( (!bJoined )  &&  (iRetry < COMMAND_RETRY_ATTEMPTS)  ) {
//    DEBUG_LOG(3, "Trying to join...") 
//      bJoined = _wifly->join(); 
//      DEBUG_LOG(3, bJoined);  
//      
//     iRetry++;
//    
//  }
//  
//  if ( bJoined ) {
//    
//    DEBUG_LOG(2, "Joined - setting addresses.");
////    char bufAddr[IP_ADDR_WIDTH];
////    
////    if (local_ip != WFSIPAddress(0,0,0,0) ){
////      DEBUG_LOG(2, "Set local IP");
////      
////      _wifly->setIP(IP_ArrayToBuffer( local_ip._address, bufAddr, IP_ADDR_WIDTH));
////    }
////    if (subnet != WFSIPAddress(0,0,0,0) ){
////      _wifly->setNetMask(IP_ArrayToBuffer(subnet._address, bufAddr, IP_ADDR_WIDTH));
////    }
////    if (gateway != WFSIPAddress(0,0,0,1) ){
////      _wifly->setGateway(IP_ArrayToBuffer(gateway._address, bufAddr, IP_ADDR_WIDTH));
////    }
////    if (dns_server != WFSIPAddress(0,0,0,1) ){
////      _wifly->setDNS(IP_ArrayToBuffer(dns_server._address, bufAddr, IP_ADDR_WIDTH));
////    }
////
////    return true;
//
//  } else {
//      DEBUG_LOG(2,"Failed to join...");
//    return false;
//  }
//}

bool WFSEthernet::setNTPServer( char* pNTPServer, float fTimeZoneOffsetHrs ) {
  
    char bufRequest[ SMALL_COMMAND_BUFFER_SIZE ];
    
    // Set NTP server, update frequency, 
    _wifly->setNTP(pNTPServer); 
    _wifly->SendCommand((char*) "set time enable 60 ", WiFlyFixedPrompts[WIFLY_MSG_PROMPT] ,bufRequest, SMALL_COMMAND_BUFFER_SIZE);
    
    // set WiFly's time sync frequency with NTP server.
    _wifly->setNTP_Update_Frequency(WIFLY_NTP_SYNC_INTERVAL);
    // Forces update of time.
    _wifly->SendCommand((char*) "time", WiFlyFixedPrompts[WIFLY_MSG_PROMPT] ,bufRequest, SMALL_COMMAND_BUFFER_SIZE);
    
    _wifly->setNTP_UTC_Offset ( fTimeZoneOffsetHrs );
    
    //setTime( _wifly->getTime() );
   // Set timezone adjustment: PST is -8h.  Adjust to your local timezone.
    
    delay(1000);
    
    
    //setSyncProvider( _wifly->getSyncTime );
    
    // Set Arduino's time sync frequency with WiFly
    //setSyncInterval( WIFLY_TIME_SYNC_INTERVAL );
    return true;
  
}

// Connection and Communication


long WFSEthernet::getDeviceStatus(){
    return _wifly->getDeviceStatus();
}

bool WFSEthernet::serveConnection(){
    return _wifly->serveConnection(WIFI_WAITING_TIME);
}


bool WFSEthernet::isConnectionOpen(){
    return _wifly->isConnectionOpen();
}

bool WFSEthernet::connect(uint8_t * addr, uint16_t port){
	char bufIP[SMALL_COMMAND_BUFFER_SIZE];
    //IP_ArrayToBuffer(   addr, (char*) &bufIP, IP_ADDR_WIDTH);
    
    return _wifly->openConnection((char *) addr, port);
}

void WFSEthernet::connect(){
    _wifly->openConnection();
}


bool WFSEthernet::disconnect(){

    return     _wifly->closeConnection(true);
}


//Standard Stuff

void WFSEthernet::flush(){
    _wifly->flush();
}

//TODO Implement a read function with a buf
uint8_t WFSEthernet::read(){
    return _wifly->read();
}
//
//uint16_t recv(SOCKET s, uint8_t *buf, uint16_t len) {
//    uint16_t ret=0;
//    char ch;
//    boolean bCollecting = true;
//    
//    while (bCollecting) {
//        if (wifi.available() > 0 ) {
//            ch = wifi.read(); 
//            buf[ret]=ch;
//            if ( ++ret == len ) {     
//                bCollecting = false;  
//            } // if received requested data length
//        } // if data available 
//    } // while collecting
//    return ret;
//}

//uint16_t sendto(const uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t port)
//{
//    uint16_t ret=0;
//    
//    // open connection, send data, close connection.
//    // might do smarter things like check if connection is already live first...
//    // and check results
//    this->connect (addr, port);
//    this->send(buf, len);
//    return ret;
//}

void WFSEthernet::write(uint8_t byte){
     _wifly->write(byte);
}

int WFSEthernet::available(){
    return _wifly->available();
}

bool WFSEthernet::SendCommand(char * cmd){
    char szRec[SMALL_COMMAND_BUFFER_SIZE];
    
    return _wifly->SendCommand(cmd, WiFlyFixedPrompts[WIFLY_MSG_PROMPT], szRec, SMALL_COMMAND_BUFFER_SIZE);
}










// IP related Functions

WFSIPAddress WFSEthernet::localIP()
{
  WFSIPAddress ret;
  char bufAddr[COMMAND_BUFFER_SIZE];
  
  _wifly->getIP(   bufAddr, COMMAND_BUFFER_SIZE );
  
    //_wifly->BufferToIP_Array(bufAddr,  ret._address);
  
  return ret;
}

WFSIPAddress WFSEthernet::subnetMask()
{
  WFSIPAddress ret;
  char bufAddr[SMALL_COMMAND_BUFFER_SIZE];
  
  _wifly->getNetMask(   bufAddr, SMALL_COMMAND_BUFFER_SIZE );
  
    //BufferToIP_Array(bufAddr,  ret._address);
  
  return ret;
}

WFSIPAddress WFSEthernet::gatewayIP()
{
  WFSIPAddress ret;
  char bufAddr[SMALL_COMMAND_BUFFER_SIZE];
  
  _wifly->getGateway(   bufAddr, SMALL_COMMAND_BUFFER_SIZE );
  
    //BufferToIP_Array(bufAddr,  ret._address);
  
  return ret;
}

WFSIPAddress WFSEthernet::dnsServerIP()
{
  WFSIPAddress ret;
  char bufAddr[SMALL_COMMAND_BUFFER_SIZE];
  
  _wifly->getDNS(   bufAddr, SMALL_COMMAND_BUFFER_SIZE );
  
    //BufferToIP_Array(bufAddr,  ret._address);
  
  return ret;
}

//WFSEthernet WFSEthernet;

/*
 * WFSEthernetClient.cpp
 * Arduino Ethernet Client class for wifi devices
 * Based on Arduino 1.0 EthernetClient class
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
//#include "WiFlySerial/WFSsocket.h"

#include "string.h"
#include "WiFlySerial/WFSEthernetClient.h"



uint16_t WFSEthernetClient::_srcport = 1024;

WFSEthernetClient::WFSEthernetClient(){
    
}

WFSEthernetClient::WFSEthernetClient(WFSEthernet * WFSE) {
    _wfs = WFSE;
}

int WFSEthernetClient::connect(const char* host, uint16_t port) {
  return connect( host, port);
}

int WFSEthernetClient::connect(WFSIPAddress ip, uint16_t port) {


  return connect(ip, port);

}

// Connect for the server
void WFSEthernetClient::connect(){
    _wfs->connect();
}

void WFSEthernetClient::write(uint8_t b) {
  return _wfs->write(b);
}

//void WFSEthernetClient::write(const uint8_t *buf, size_t size) {
//  _wfs->write(
//}

int WFSEthernetClient::available() {
    
    // WiFly supports single connection at present so only one socket.
    // TODO: support multiple sockets, or at least appear to do so.
   //    return _wfs->available(_sock);
    return _wfs->available();
}

int WFSEthernetClient::read() {
	return _wfs->read();  
}

//
int WFSEthernetClient::peek() {
    return -1;
}


// flush
// permits outgoing characters to complete sending, per Arduino 1.0 Serial implementation.
// Parameters: None
// 

void WFSEthernetClient::flush() {
  _wfs->flush();

// drain incoming characters  
//  while (available())
//    read();
}

void WFSEthernetClient::stop() {
   // attempt to close the connection gracefully (send a FIN to other side)
    
  _wfs->disconnect();

}

bool WFSEthernetClient::connected(){
	return _wfs->isConnectionOpen();
}

// status
// Provides Client status
//
// Parameters: none
// Returns:
// one of:
// 0      CLOSED
// 0x17      Connected
//
bool WFSEthernetClient::status() {
    //TODO überhaupt connected?!
  return _wfs->isConnectionOpen() ;
}

// the next function allows us to use the client returned by
// EthernetServer::available() as the condition in an if-statement.

WFSEthernetClient::operator bool() {
//  return _sock != MAX_SOCK_NUM;
   return this->status();
}


uint8_t WFSEthernetClient::devicestatus() {
  return _wfs->getDeviceStatus();
}


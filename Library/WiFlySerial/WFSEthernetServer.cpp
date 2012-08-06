/*
 * WFSEthernetServer.cpp
 * Arduino Ethernet Server class for wifi devices
 * Based on Arduino 1.0 EthernetServer class
 * 
 * Credits:
 * First to the Arduino Ethernet team for their model upon which this is based.
 * Modifications: 
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


#include <string.h>



#include "WiFlySerial/WFSEthernetServer.h"

WFSEthernetServer::WFSEthernetServer(uint16_t port, long profile)
{
	_port = port;
    _ServerProfile = profile;
}


// begin
// Sets basics for device configuration
void WFSEthernetServer::begin(WFSEthernet * WiFly)
{
	_wifi = WiFly;
    //Important!
    _wifi->SendCommand( "set u m 0x1") ;
    //TODO optional setting to change the TCP/IP packacke size
    _wifi->SendCommand( "set comm idle 30") ;
    _wifi->SendCommand( "set comm time 100") ;
    _wifi->SendCommand( "set comm size 255") ;
    
    this->setProfile();
}
// setProfile
// Configures server to respond in manner specified
// Assumes wifi device is semi-automnous.
//
// Parameters:
// serverProfile      bit-mapped flag of profile options

long WFSEthernetServer::setProfile() {
    
    
    // Defaults as according to device.
    // Settings may be residual values from prior saved sessions.
    if ( _ServerProfile & ES_DEVICE_DEFAULT) {
        // ToDo Set to default somehow (factory reset?)
    }
    
    if ( _ServerProfile & ES_HTTP_SERVER ) {
        // No *HELLO* on connection - confuses browsers
        _wifi->SendCommand( "set comm remote 0");      
        // Send packet on each tab character issued
        _wifi->SendCommand("set comm match 0x9");   
    }
    // Telnet response setup
    if ( _ServerProfile & ES_TELNET_SERVER)  {
        //TODO Telnet
    }
    
    // UDP not implemented yet on CS libraries
    if ( _ServerProfile & ES_UDP_SERVER ) {
        // ToDo UDP
        
    }
    
    return _ServerProfile;    
}

void WFSEthernetServer::accept()
{

  _wifi->serveConnection();

}


// returns a WFSEthernetClient using an available socket.
// WiFly has one socket so ... use it.
WFSEthernetClient WFSEthernetServer::available()
{
  accept();
  WFSEthernetClient client((this->_wifi));
  return client;
  
//  return WFSEthernetClient(MAX_SOCK_NUM);
}

void WFSEthernetServer::write(uint8_t b) 
{
  _wifi->write(b);
}

// TODO recognize that here write includes the size! 
//void WFSEthernetServer::write(const uint8_t *buffer, size_t size) 
//{
//  
//  return _wifi->write(buffer, size);
//}

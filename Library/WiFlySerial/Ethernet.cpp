#include "w5100.h"
#include "Ethernet.h"
#include "Dhcp.h"

// XXX: don't make assumptions about the value of MAX_SOCK_NUM.
uint8_t EthernetClass::_state[MAX_SOCK_NUM] = { 
  0, 0, 0, 0 };
uint16_t EthernetClass::_server_port[MAX_SOCK_NUM] = { 
  0, 0, 0, 0 };




IPAddress EthernetClass::localIP()
{
  IPAddress ret;
  W5100.getIPAddress(ret.raw_address());
  return ret;
}

IPAddress EthernetClass::subnetMask()
{
  IPAddress ret;
  W5100.getSubnetMask(ret.raw_address());
  return ret;
}

IPAddress EthernetClass::gatewayIP()
{
  IPAddress ret;
  W5100.getGatewayIp(ret.raw_address());
  return ret;
}

IPAddress EthernetClass::dnsServerIP()
{
  return _dnsServerAddress;
}

EthernetClass Ethernet;

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
- Can listen on multiple ports.
- most settings assumed volatile; fetched from WiFly where reasonable.

Expected pattern of use:
begin
issue commands, such as set SSID, passphrase etc
exit command mode / enter data mode
listen for web activity
Open a TCP connection to a peer
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

Copyright GPL 2.1 Tom Waldock 2011, 2012
*/

#include <string.h>
#include "stdlib.h"
#include <time.h>
#include "WiFlySerial/WiFlySerial.h"
#include "WiFlySerial/Debug.h"



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
const char  s_WIFLYDEVICE_SET_UART_BAUD[] __FLASH__ ="set uart instant ";
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


// Utility Functions
//
// WFSIPArrayToStr
// Converts IPArray to a character string representation for WiFlySerial
//
// pIP    pointer to array of 4 ints representing ip address e.g. {192,168,1,3}
// pStr   Destination buffer of at least 16 characters "192.168.1.3"
// returns 0 on success
//
// Note: could be enhanced to support IPv6

// Convert a Buffer holding an IP address to a byte array.
// Minimal safety checks - be careful!
uint8_t* BufferToIP_Array(char* pBuffer, uint8_t* pIP) {
    
    char* posStart=0;
    char* posEnd=0;
    char alphabuf[IP_ADDR_WIDTH];
    
    posStart = pBuffer;
    for (int i = 0; i<UC_N_IP_BYTES ; i++) {
        memset(alphabuf,'\0',IP_ADDR_WIDTH);
        posEnd = strchr(posStart,'.');
        if (posEnd == NULL) {
            posEnd = strchr(posStart,'\0');
        }
        strncpy(alphabuf, posStart, posEnd-posStart);
        
        pIP[i] = (uint8_t) (int)( alphabuf );
        // Start looking one after last dot.
        posStart = posEnd +1;
    }
    return pIP;
}

// Convert a Buffer holding an IP address to a byte array.
// Minimal safety checks - be careful!
char* IP_ArrayToBuffer( const uint8_t* pIP, char* pBuffer, int buflen) {
    
    memset (pBuffer,'\0',buflen);
    
    for (int i =0; i< UC_N_IP_BYTES; i++) {
        itoa( (int) pIP[i], strchr(pBuffer,'\0'), 10);
        if (i < UC_N_IP_BYTES -1 ) {
            strcat(pBuffer, ".");
        }
    }
    
    
    return pBuffer;
}


void reverse(char *result, char * string) {
    int x = strlen(string)-1;
    for(int y = x; y >= 0; y--) {
        result[x-y] = string[y];
    }
}


// http://www.jb.man.ac.uk/~slowe/cpp/itoa.html
char * itoa( int value, char* result, int base ) {
    // check that the base if valid
    if (base < 2 || base > 16) { *result = 0; return result; }
	
    char* out = result;
    int quotient = abs(value);
	
    do {
        const int tmp = quotient / base;
        *out = "0123456789abcdef"[ quotient - (tmp*base) ];
        ++out;
        quotient = tmp;
    } while ( quotient );
	
    // Apply negative sign
    if ( value < 0) *out++ = '-';
	
    reverse( result, out );
    //*out = 0;
    return result;
}

/*
 Command and Response
 WiFly provides one of three results from commands:
 1) ERR: Bad Args  , from malformed commands.
 2) AOK , from accepted commands
 3) nothing, after an inquiries' response.
 
 Some commands will provide specific messages
 e.g. join  has a possible result of
 mode=WPA1 SCAN OK followed by 'Associated!' and by ip values.
 (bad SSID)  mode=NONE FAILED
 (bad pwd)  mode=WPA1 SCAN OK followed by 'Disconn ...  AUTH-ERR'
 and followed by 'Disconn from <SSID>'
 
 after a successful join, a 'cr' is needed to get the prompt
 
 The 'command prompt' is currently the version number in angle-brackets e.g. <2.21>
 
 
 */
void WiFlySerial::printInternal(){
	
    DEBUG_LOG(3, "szWiFlyPrompt");
    DEBUG_LOG(3, _szWiFlyPrompt);
    DEBUG_LOG(3, "_fStatus");
    DEBUG_LOG(3, _fStatus);
    DEBUG_LOG(3, "_bWiFlyInCommandMode");
    DEBUG_LOG(3, _bWiFlyInCommandMode);
    DEBUG_LOG(3, "_bWiFlyConnectionOpen");
    DEBUG_LOG(3, _bWiFlyConnectionOpen);
    DEBUG_LOG(3, "_pControl");
    DEBUG_LOG(3, _pControl);
    DEBUG_LOG(3, "_iRemotePort");
    DEBUG_LOG(3, _iRemotePort);
    DEBUG_LOG(3, "_iLocalPort");
    DEBUG_LOG(3, _iLocalPort);
}

WiFlySerial::WiFlySerial() {

    // Set initial values for flags.
    // On Maple startup, WiFly state not known.
    _bWiFlyInCommandMode = false;
    _bWiFlyConnectionOpen = false;
    _fStatus = WIFLY_STATUS_OFFLINE ;
    //  strcpy(this->_szWiFlyPrompt, WiFlyFixedPrompts[WIFLY_MSG_PROMPT2] );  // ">"
	DEBUG_LOG(3, "szWiFlyPromt at instance");
	DEBUG_LOG(3, this->_szWiFlyPrompt);
    _iLocalPort = WIFLY_DEFAULT_LOCAL_PORT;
    _iRemotePort = WIFLY_DEFAULT_REMOTE_PORT;

    
    // default is UTC timezone
    lUTC_Offset_seconds = 0;
    
    _pControl = WiFlyFixedPrompts[WIFLY_MSG_CLOSE] ;
    

}


// begin
// Initializes WiFly interface and starts communication with WiFly device.
//
// Parameters: none.
// Returns: true on initialize success, false on failure.
bool WiFlySerial::begin(SpiUartDevice * TheSpi) {

    char szCmd[SMALL_COMMAND_BUFFER_SIZE];
    
    
    //// TODO: set default uart transmission speed to same as WiFly default speed.
    //uart.begin(WIFLY_DEFAULT_BAUD_RATE);
    
    
    _uart = TheSpi;
    
    //Device may or may not be:
    // awake / asleep
    // net-connected / connection lost
    // IP assigned / no IP
    // in command mode / data mode
    // in known state / confused
    // therefore reboot
    
    // todo in the final emove the commend
    //reboot();


    this->_bWiFlyInCommandMode = false;
	    
    StartCommandMode(szCmd, SMALL_COMMAND_BUFFER_SIZE);
    
    // TODO just set some initial stuff
    // like ports from the init
    // like time 
    // like baud rate increase?!
    
    //   // set baud rate
    //   bStart = SendCommand( GetBuffer_P(STI_WIFLYDEVICE_SET_UART_MODE, szCmd, SMALL_COMMAND_BUFFER_SIZE),
    //                          WiFlyFixedPrompts[WIFLY_MSG_AOK],
    //                          szResponse,
    //                          COMMAND_BUFFER_SIZE );
    //   bStart = SendCommand( GetBuffer_P(STI_WIFLYDEVICE_SET_UART_BAUD, szCmd, SMALL_COMMAND_BUFFER_SIZE),
    //                          WiFlyFixedPrompts[WIFLY_MSG_AOK],
    //                         szResponse,
    //                         COMMAND_BUFFER_SIZE );
	
    

    //Determine the expression between the < > and if unsuccessful say we have a problem!
	if (! GetCmdPrompt() ) {
        DEBUG_LOG(1, "Got a Problem with starting the WiFly");
        return false;
        //  TODO maybe we should reboot and do all the stuff again ;)
	}
    
    return true;
}



// ScanForPattern
//
// General-purpose stream watcher.
// Monitors incoming stream until given prompt is detected, or error conditions, or until timer expired
////
// Parameters
// ResponseBuffer    buffer for WiFly response
// bufsize           size of buffer
// pExpectedPrompt   Marker to find
// bCollecting       true: collect chars in buffer UNTIL marker found, false: discard UNTIL marker found
// WaitTime          Timeout duration to wait for response
// bPromptAfterResult true: version prompt after result, false: version prompt precedes results (scan, join).
//
// Returns: (see .h file)  OR-ed flags of the following
// WiFly Responses:
//#define PROMPT_NONE 0
//#define PROMPT_EXPECTED_TOKEN_FOUND 1
//#define PROMPT_READY 2
//#define PROMPT_CMD_MODE 4
//#define PROMPT_AOK 8
//#define PROMPT_OTHER 16
//#define PROMPT_CMD_ERR 32
//#define PROMPT_TIMEOUT 64
//#define PROMPT_OPEN 128
//#define PROMPT_CLOSE 256

int WiFlySerial::ScanForPattern( char* responseBuffer, const int buflen,  const char *pExpectedPrompt, const bool bCollecting,  const unsigned long WaitTime, const bool bPromptAfterResult) {
    
    byte iPromptFound = PROMPT_NONE;
    char chResponse = 'A';
    int  bufpos = 0;
    int  bufsize = buflen -1;  //terminating null for bufsize
    bool bWaiting = true;
    memset (responseBuffer, '\0', bufsize);
    
    
    WiFlyFixedPrompts[WIFLY_MSG_EXPECTED] = (char*) pExpectedPrompt; //??
	WiFlyFixedPrompts[WIFLY_MSG_PROMPT] = (char*) this->_szWiFlyPrompt;   //??
    char* pFixedCurrent[N_PROMPTS];

    DEBUG_LOG(4, "Beeing in Scan for Pattern");
    DEBUG_LOG(4, pExpectedPrompt);
    
    for (int i=0; i < N_PROMPTS; i++) {
        pFixedCurrent[i] = WiFlyFixedPrompts[i];
    }
    
    unsigned long TimeAtStart = millis()  ;  // capture current time
    
    while ( bWaiting ) {
        if ( _uart->available() > 0 ) {
            chResponse = _uart->read();
            DEBUG_LOG(5, chResponse);
            // if we capture the response
            // if the supplied buffer is too small we just capture the beginning
            if ( bCollecting && bufpos < buflen) {
                responseBuffer[bufpos++] = chResponse;
            }
            
            // Going through all possible Responds
            for (int iFixedPrompt = 0; iFixedPrompt < N_PROMPTS; iFixedPrompt++ ) {
               
                if ( chResponse == *pFixedCurrent[iFixedPrompt] ) {
                    // deal with 'open' and 'scan' version-prompt appearing BEFORE result; ignore it
                    if ( (!bPromptAfterResult) && (iFixedPrompt == WIFLY_MSG_PROMPT || iFixedPrompt == WIFLY_MSG_PROMPT2) /* standard version-prompt */  ) {
                        bWaiting = true;
                        DEBUG_LOG(5, "beeing here");
                        iPromptFound |= PROMPT_READY;
                    }
                    //standard case
                    else {
                        // going one character further in the string at the next character
                        bWaiting = ( *(++pFixedCurrent[iFixedPrompt]) == '\0' ? false : true ) ; // done when end-of-string encountered.
						// we reached the end of a response and put it in the iPrompt found 
                        if (!bWaiting) {
                            iPromptFound |= WiFlyFixedFlags[iFixedPrompt];  // if a prompt found then grab its flag.
                            // if we find the <XXX> promopt we might wand to proceed if we havent found our token we are looking for…
                        	if (iPromptFound & PROMPT_READY ) {
                                DEBUG_LOG(5, "Found <XXX Prompt> -> Proceed");
                                bWaiting = true;
                                // putting even the string at the beginning, maybe it comes floats by again…
                                pFixedCurrent[iFixedPrompt] = WiFlyFixedPrompts[iFixedPrompt];  // not 
                            }
                            
                        }
                        
                    } // handle prompt-BEFORE-result case
                }
                else {
                    pFixedCurrent[iFixedPrompt] = WiFlyFixedPrompts[iFixedPrompt];  // not next char expected; reset to beginning of string.
                } // if tracking expected response
            }
            // If the *OPEN* signal caught then a connection was opened.
            if (iPromptFound & (PROMPT_OPEN | PROMPT_OPEN_ALREADY)  ) {
                _bWiFlyConnectionOpen = true;
                this->_bWiFlyInCommandMode = false;
                iPromptFound &= (!WiFlyFixedFlags[WIFLY_MSG_CLOSE]);  // clear prior close
            }
            
            // If the *CLOS* signal caught then a connection was closed
            // and we dropped into command mode
            if (iPromptFound & PROMPT_CLOSE ) {
                _bWiFlyConnectionOpen = false;
                this->_bWiFlyInCommandMode = true;
                iPromptFound &= (!WiFlyFixedFlags[WIFLY_MSG_OPEN]);  // clear prior open
            }
            // We found our token and can exit...
            if (iPromptFound & PROMPT_EXPECTED_TOKEN_FOUND) {
                DEBUG_LOG(4, "Expected found");
                bWaiting = false;
                
            }
            
        } // if anything in uart

        // did we time-out?
        if ( (millis() - TimeAtStart) >= WaitTime) {
            bWaiting = false;
        }
    }  // while waiting for a line
       // could capture and compare with known prompt
    return (int) iPromptFound;
    
} // ScanForPattern



// Start Command Mode
// 
// Attempt up till the ATTN_WAIT_TIME passes times
// test is "Get a command prompt matching results of 'ver' command".
// if InCommand mode, try a 'cr'.  
// If no useful result, assume not actually in command mode, force with $$$
// 
// Returns true for Command mode entered, false if not (something weird afoot).


bool WiFlySerial::StartCommandMode(char* pBuffer, const int bufSize) {
    byte iPromptResult = PROMPT_NONE; 	// 0
    bool bWaiting = true;				//	either time out or be successfull indicator
    int nTries = 0;						// Attemps to get into Command Mode
	memset(pBuffer, '\0', bufSize);		// clearing the supplied buffer
    
    
    unsigned long TimeOutTime = millis() + ATTN_WAIT_TIME;

    // Do this until we either time out or we got into cmd mode
    while  (bWaiting ) {
        _uart->flush();         
        // We believe we are not in cmd Mode and will try $$$
        if ( !this->_bWiFlyInCommandMode) {
            DEBUG_LOG(3, "Try to get into command mode");
            // Put in the Delay time, Send $$$ , wait a moment, look for CMD
            delay(COMMAND_MODE_GUARD_TIME );
            _uart->print( (WiFlyDevice_string_table[STI_WIFLYDEVICE_ATTN]) );
            delay(COMMAND_MODE_GUARD_TIME  );
			
            //if we didn't believe we are int cmd mode but we are? We send a new line and check for different answers
            _uart->println();
            
            iPromptResult = ScanForPattern( pBuffer, bufSize, "CMD", true, ATTN_WAIT_TIME);       
            DEBUG_LOG(4, "pBuffer");
            DEBUG_LOG(4, pBuffer);
            // TODO
            if ( iPromptResult & ( PROMPT_EXPECTED_TOKEN_FOUND | PROMPT_READY | PROMPT_CMD_MODE | PROMPT_CMD_ERR ) ) {
                this->_bWiFlyInCommandMode = true;
                bWaiting = false;
            }
        //assuming we are in Command mode but have to prove it with \r\n or "ver" call    
        } else {
            DEBUG_LOG(3, "Assuming we are already in the command mode; proof it");
            //this->_bWiFlyInCommandMode = false;
            
            // think we are in a command-mode - try a cr, then add a version command to get through.
            // send a ver + cr, should see a prompt.
            _uart->println();
            
            // After 3 Attemps we send a "ver" over and see if we catch anything then 
            if (nTries >= 2)  {
                DEBUG_LOG(3, "Need so send ver for determining command mode");
            	_uart->println( WiFlyDevice_string_table[STI_WIFLYDEVICE_VER]) ;
			}
            
            // there should be a <2.XX> in the response
            iPromptResult = ScanForPattern( pBuffer, bufSize, this->_szWiFlyPrompt, false);
            // could have timed out, or have *READY*, CMD or have a nice CR.
            
            if ( iPromptResult & ( PROMPT_EXPECTED_TOKEN_FOUND | PROMPT_AOK | PROMPT_READY |PROMPT_CMD_MODE |PROMPT_CMD_ERR ) ) {
                this->_bWiFlyInCommandMode = true;
                bWaiting = false;
            }
            if (nTries >=5) {
                DEBUG_LOG(2, "We assumed cmd mode but this was wrong!");
                this->_bWiFlyInCommandMode = false;
                // Maybe increase the timeout time now…
            }
            
        } //  else in in command command mode
        
        // Stop if we are above the TimeOutTime
        if ( millis() >= TimeOutTime ) {
            bWaiting = false;
        }
        nTries++;
    } // while trying to get into command mode
	DEBUG_LOG(4, "In CMD mode?");
    DEBUG_LOG(4, _bWiFlyInCommandMode);
    return this->_bWiFlyInCommandMode;
}


// GetCmdPrompt
// Obtains the WiFly command prompt string for use by other command functions.
// Parameters: None
// Sets global szWiFlyPrompt
// Returns command prompt on success or empty string on failure
bool WiFlySerial::GetCmdPrompt () {
    char responseBuffer[RESPONSE_BUFFER_SIZE];
    
    // Get in command mode
    if ( StartCommandMode(responseBuffer, RESPONSE_BUFFER_SIZE)  ) {
        // send a "ver"
        _uart->println( WiFlyDevice_string_table[STI_WIFLYDEVICE_VER] );
        // save everything to the response Buffer
        ScanForPattern(responseBuffer, RESPONSE_BUFFER_SIZE, WiFlyFixedPrompts[WIFLY_MSG_PROMPT2], true, COMMAND_MODE_GUARD_TIME);
        // and do some string manipulation / search
        char* pPromptStart = strrchr(responseBuffer, '<') ;
        char* pPromptEnd = strrchr (responseBuffer, '>');
        
        // if search was successfull we extract it and put it in our _szWiflyPrompt 
        if ( (pPromptStart < pPromptEnd ) && pPromptStart && pPromptEnd) {
            strncpy(_szWiFlyPrompt, pPromptStart , (size_t) (pPromptEnd - pPromptStart)+1 );
            _szWiFlyPrompt[(pPromptEnd - pPromptStart)+1] = '\0';
        }
    }
    
    if ( strlen (_szWiFlyPrompt) > 1 ) {
        DEBUG_LOG(4, "CmdPrompt:" );
        DEBUG_LOG(4, _szWiFlyPrompt);
        return true;
    } 
    
    return false;  
}

bool WiFlySerial::reboot() {
    char responseBuffer[RESPONSE_BUFFER_SIZE];
    bool success = false;    
    DEBUG_LOG(1, "Entered softwareReboot");
        
    for (int retryCount = 0;
         retryCount < SOFTWARE_REBOOT_RETRY_ATTEMPTS;
         retryCount++) {
        
        // TODO: Have the post-boot delay here rather than in enterCommandMode()?
        // StartCommandMode(responseBuffer, RESPONSE_BUFFER_SIZE);
        success = SendCommand(WiFlyDevice_string_table[STI_WIFLYDEVICE_REBOOT], "*READY*", responseBuffer, RESPONSE_BUFFER_SIZE, true, WIFLY_RESTART_WAIT_TIME);
                    
        if (success) {
            delay(WIFLY_RESTART_WAIT_TIME);
            return true;
        } 
    }
    return false;
}



// SendCommand
// Issues a command to the WiFly device
// Captures results in Returned result
// 
//
// Parameters: 
// Command          The inquiry-command to send
// SuccessIndicator String to indicate success 
// pResultBuffer    A place to put results of the command
// bufsize          Length of the pResultBuffer
// bCollecting      true = collect results, false=ignore results.
// iWaitTime        Time in milliseconds to wait for a result.
// bClear           true = drain any preceeding and subsequent characters, false=ignore
// bPromptAfterResult true=commands end with a version-prompt, false=version-prompt precedes results.
//
// Returns true on SuccessIndicator presence, false if absent.
bool WiFlySerial::SendCommand(const char *pCmd, const char *SuccessIndicator, char* pResultBuffer, const int bufsize, 
                                 const bool bCollecting, const  unsigned long iWaitTime, const bool bClear, const bool bPromptAfterResult) {
    
    bool bCommandOK = false;

    int iResponse = 0;
    int iTry = 0;
    
    
    // clear out leftover characters coming in
    if ( bClear ) {
		_uart->flush();
		// maybe we could show it in the debug?!
    } 
    
    
    
    
    // Check the command Mode
    if ( StartCommandMode(pResultBuffer, bufsize) ) {
        

        while ( ((iResponse & PROMPT_EXPECTED_TOKEN_FOUND) != PROMPT_EXPECTED_TOKEN_FOUND) && iTry < COMMAND_RETRY_ATTEMPTS ) {
			_uart->flush();
            DEBUG_LOG(3, "Sending Command:");
            DEBUG_LOG(3, pCmd );
            DEBUG_LOG(3, "Success Indicator:");
            DEBUG_LOG(3, SuccessIndicator);
            // Send the Command
            _uart->println( pCmd ) ;

            iResponse = ScanForPattern( pResultBuffer, bufsize, SuccessIndicator, bCollecting, iWaitTime, bPromptAfterResult );   
            
//			DEBUG_LOG(3,"Try#:");
//            DEBUG_LOG(3, iTry ); 
//            DEBUG_LOG(3," iRes:");
//            DEBUG_LOG(3,iResponse);
//            DEBUG_LOG(3," pResult:");
//            DEBUG_LOG(3,pResultBuffer);
            iTry++;
            DEBUG_LOG(4, "Sending it xx Times:");
            DEBUG_LOG(5, iTry);
            DEBUG_LOG(4, iResponse);
        }
        
    }
    
    
    // Found the response and ommit all the rest
    if ( bClear ) {
        _uart->flush();
    } // clear out leftover characters
    
    bCommandOK = ( ((iResponse & PROMPT_EXPECTED_TOKEN_FOUND) == PROMPT_EXPECTED_TOKEN_FOUND) ? true : false );
    
    return bCommandOK;
}

// getDeviceStatus
// Refreshes device status flags
//
// WiFly 'show connection' command gives a result in hex 8XYZ as a string.
//
// Parameters
// None.
// Returns true on success, false on failure.
// 
long WiFlySerial::getDeviceStatus() {
    
    char bu_fStatus[INDICATOR_BUFFER_SIZE];
    memset(bu_fStatus,'\0',INDICATOR_BUFFER_SIZE);
    // place leading fixed '8' into first position
    bu_fStatus[0]='8';
    
    ExtractDetail( WiFlyDevice_string_table[STI_WIFLYDEVICE_GET_STATUS], &bu_fStatus[1], INDICATOR_BUFFER_SIZE - 1, WiFlyDevice_string_table[STI_WIFLYDEVICE_GET_STATUS_IND], WiFlyDevice_string_table[STI_WIFLYDEVICE_RETURN] ) ;
    DEBUG_LOG(3, "Get Device Status");
    DEBUG_LOG(3, bu_fStatus);
    _fStatus = strtol(bu_fStatus, (char**)0, 16);
    return _fStatus;
    
}

// ExtractDetailIdx
// Returns substring from a response.  Indexes point to command, search string and terminator strings.
//
// Parameters: 
// idxCommand     StringID of command
// pDetail        pointer to destination buffer
// buflen         length of destination buffer
// idxSearch      StringID to extract AFTER
// idxStop        StringID to extract UNTIL
//
// Returns:       pointer to destination buffer
char* WiFlySerial::ExtractDetailIdx(const int idxCommand, char* pDetail, int buflen, const int idxSearch, const int idxStop) {
    char bufCmd[COMMAND_BUFFER_SIZE];
    char bufSearch[INDICATOR_BUFFER_SIZE];
    char bufStop[INDICATOR_BUFFER_SIZE];
    
    strcpy(bufCmd,    WiFlyDevice_string_table[idxCommand]);
    strcpy(bufSearch, WiFlyDevice_string_table[idxSearch]);
    strcpy(bufStop,   WiFlyDevice_string_table[idxStop]);
    
    return ExtractDetail( bufCmd, pDetail, buflen, bufSearch, bufStop);
    
}


// ExtractDetail
// Returns substring from a response.
//
// Parameters: 
// pCommand       pointer to command
// pDetail        pointer to destination buffer
// buflen         length of destination buffer
// pFrom          String to extract AFTER
// pTo            String to extract UNTIL
//
// Returns:       pointer to destination buffer
char* WiFlySerial::ExtractDetail(const char* pCommand, char* pDetail, int buflen, const char* pFrom, const char* pTo) {
    char* pEndToken = NULL;
    
    //send command and ignore results up to after pFrom
    SendCommand(pCommand,  
                (char*) pFrom,
                pDetail,
                buflen,
                false,  DEFAULT_WAIT_TIME, false  );
    
    // then collect results up to after pTo.
    ScanForPattern(pDetail, buflen, pTo, true,  DEFAULT_WAIT_TIME  );                           
    
    // trim result to not include end token.
    DEBUG_LOG(4, "After Scan For Pattern");
    DEBUG_LOG(4, pDetail);
    if ( (pEndToken = strstr(pDetail, pTo) ) != NULL) {
        *pEndToken = '\0';
    }
    while (*(++pEndToken) != '\0') {
        *pEndToken = '\0';
    }
    // clear buffer of remaining characters
    // ScanForPattern(strchr(pDetail,'\0')+1, buflen - strlen(pDetail) -1 , "\0\0", false,  DEFAULT_WAIT_TIME  );                           
    //   while ( uart.available() > 0 ) {
    //     ch = uart.read();
    //     DEBUG_LOG(3, ch);
    //   }
    //
    
    return pDetail;
}

// TODO: Just one of them!

// SendInquiry
// Inquiries provide a device setting result, terminated with a command prompt.
// No specific 'ok/fail' result shown, only ERR or requested response.
// Results placed into global responsebuffer
//
// Parameters: 
// Command        The inquiry-command to send
// pBuffer        pointer to a buffer for the response
// bufsize        size of the buffer
//
// Returns true on command success, false on failure.
bool WiFlySerial::SendInquiry( char *Command, char* pBuffer, const int bufsize) {
    
    return  SendCommand(Command,  _szWiFlyPrompt, pBuffer, bufsize, true);
    
}
// SendInquiry
// Inquiries provide a device setting result, terminated with a command prompt.
// No specific 'ok/fail' result shown, only ERR or requested response.
// Results placed into global responsebuffer
//
// Parameters: 
// Command        The inquiry-command to send
//
// Returns true on command success, false on failure.
bool WiFlySerial::SendInquirySimple( char *Command ) {
    char InquiryBuffer[RESPONSE_BUFFER_SIZE];
    
    // should trim to returned result less ExpectedPrompt
    return SendCommand(Command,  _szWiFlyPrompt, InquiryBuffer, RESPONSE_BUFFER_SIZE, true);;
    
}



// exitCommandMode
// Exits from WiFly command mode.
//
// Watch the NSS for further traffic.
//
// Parameters: 
// None
// Returns true on exit cmd mode, false on failure -> probably still in cmd Mode.
bool WiFlySerial::exitCommandMode() {
    
   // exit command is short
   // exit Prompt is short:  EXIT (which looks like 'exit' but in upper case).
    char szResponse[INDICATOR_BUFFER_SIZE]; // small buffer for result
    char szSuccIndicator[INDICATOR_BUFFER_SIZE];
	//have to coppy it that we can use strupr 
    strcpy(szSuccIndicator, WiFlyDevice_string_table[STI_WIFLYDEVICE_LEAVE_CMD_MODE]);
    DEBUG_LOG(3, "In ExitCmdMode");
    _bWiFlyInCommandMode = !SendCommand( WiFlyDevice_string_table[STI_WIFLYDEVICE_LEAVE_CMD_MODE] , 
                                        strupr(szSuccIndicator),
                                       szResponse, INDICATOR_BUFFER_SIZE, false ); 
    return !_bWiFlyInCommandMode;
}


// showNetworkScan
// Displays list of available WiFi networks.
//
// Parameters: 
// pNetScan    Buffer for scan results (should be large)
// buflen      length of buffer
char* WiFlySerial::showNetworkScan( char* pNetScan, const int buflen) {
    
    SendCommand("scan","'", pNetScan, buflen, true, JOIN_WAIT_TIME, true, false) ;
    
    return pNetScan;
    
}


// setProtocol
// Sets WiFly's communication protocol
// TCP or UDP
//
// Parameters:
// iProtocol      Hex value for protocol - bit mapped values. 
// Returns true for success, false for failure or error in call
//
// Note:
// Switching from one to the other requires a reboot of the WiFly.
// For UDP, set all other settings first (unlike example in manual) ; UDP traffic will start upon WiFly reboot.
// This version does not attempt to determine current mode. Reboot forced.
bool WiFlySerial::setProtocol( unsigned int iProtocol) {
    
    bool bOk = false;
    char szReturn[SMALL_COMMAND_BUFFER_SIZE];
    unsigned int iMode = 0, iTmp; 
    char bufMode[10];  
    
    itoa( iProtocol, bufMode, 10);
    
    bOk =  issueSetting( STI_WIFLYDEVICE_SET_PROTOCOL, bufMode );
    
    // Save settings    
    bOk = SendCommand(WiFlyDevice_string_table[STI_WIFLYDEVICE_SAVE] , WiFlyFixedPrompts[WIFLY_MSG_AOK], szReturn, SMALL_COMMAND_BUFFER_SIZE );
    reboot();
    
    // Allow WiFly to restart
    delay (WIFLY_RESTART_WAIT_TIME);
    
    return bOk;
}



// openConnection
// Opens a TCP connection to the provided URL and port (defaults to 80)
//
// Parameters:
// pURL      IP or dns name of server to connect to.
// iWaitTime Time to wait for connection
//
// Returns: true on success, false on failure.
// Remote Port number is set through SetRemotePort
// Note that opened ports can be closed externally / lost connection at any time.
// Opening a connection switches to Data mode from Command mode.
//
// Note: Open and Scan each generate a version-prompt BEFORE results, not after.
bool WiFlySerial::openConnection(const char* pURL, const unsigned long iWaitTime) {
    char bufOpen[INDICATOR_BUFFER_SIZE];
    char bufCommand[COMMAND_BUFFER_SIZE];
    
    memset (bufCommand, '\0', COMMAND_BUFFER_SIZE);
    strcpy(bufCommand, WiFlyDevice_string_table[STI_WIFLYDEVICE_OPEN]);
    strcat (bufCommand, pURL);
    strcat (bufCommand, " ");
    itoa( _iRemotePort, strchr(bufCommand, '\0'), 10);
    DEBUG_LOG(3, "openConnection:");
    DEBUG_LOG(3, bufCommand);
    _bWiFlyConnectionOpen = SendCommand(bufCommand,WiFlyFixedPrompts[WIFLY_MSG_OPEN], bufOpen, INDICATOR_BUFFER_SIZE, false, iWaitTime , true, false); 
    
    
    return _bWiFlyConnectionOpen;
    
}

// Opens Connection for server mode

void WiFlySerial::openConnection(){
    _bWiFlyConnectionOpen = true;
}

// closeConnection
// closes an open connection
//
// Parameters:
// bSafeClose	   (default) Use slow 'safe close'
//                 false if high confidence connection is in fact open, 
//				   and can tolerate occasional errors if connection
//                 closes sooner than expected.
//                 
// returns   true on close, false on failure.
// Side effects: _bWiFlyConnectionOpen should become false
//          and  _bWiFlyInCommandMode should become true.
//
// Notes:
// 1. When closed via a command the WiFly state is in command mode.
// 2. External connection close leaves WiFly in data mode
// 3. Closing a closed connection results in an error.
// 4. External connection close could happen at any time.
// 5. getDeviceStatus() gives actual WiFly status at the moment of calling.
// 6. When opened via listening, info from previous getDeviceStatus() is out-of-date.
// 7. The 'close' command results with a command-prompt, no 'AOK' or similar.
//     then, a *CLOS* signal appears.
//
// returns true on closure, false on failure to close.
// 
bool WiFlySerial::closeConnection(bool bSafeClose) {
    // if a connection is open then close it.
    
    DEBUG_LOG(3, "in close Connection");
    if ( _bWiFlyConnectionOpen ) {
        // first see if connection is *STILL* open. 
      
        bool bDoClose = true;
        
        // repeat until closed...
        while ( _bWiFlyConnectionOpen ) {
            if (bSafeClose) {
                //getDeviceStatus(); is done in isTCPConnected
                if ( isTCPConnected() ) {
                    bDoClose = true;
                } else {
                    bDoClose = false;
                    _bWiFlyConnectionOpen = false; // should be redundant as a caught *CLOS* will set this to false.
                }
            } // if doing safe close
            if ( bDoClose ) {
                    DEBUG_LOG(3, "Close with cmd mode");
                //drain();
                char bufCmd[INDICATOR_BUFFER_SIZE];
                char bufClose[INDICATOR_BUFFER_SIZE];
                memset( bufCmd, '\0', INDICATOR_BUFFER_SIZE);
                memset( bufClose, '\0', INDICATOR_BUFFER_SIZE);
                
                // close command response is a prompt, then a *CLOS* signal after.
                _bWiFlyConnectionOpen = !SendCommand(WiFlyDevice_string_table[STI_WIFLYDEVICE_CLOSE],"*CLOS*",    bufClose,    INDICATOR_BUFFER_SIZE, true, DEFAULT_WAIT_TIME , false);
                DEBUG_LOG(3,  bufCmd );
                DEBUG_LOG(3,  bufClose );
                
                //drain();
                
                _bWiFlyInCommandMode = exitCommandMode();

            }
        } // while
    } else {
        // closed externally already.
        // No change to bWiFlyCommandMode. 
        return true;
    }
    return true;
}

// For the server Manual driving of the _bWiFlyconnectionOpen variable
bool WiFlySerial::closeConnection(){
   	DEBUG_LOG(2, "_bWiFlyConOPen false");
    _bWiFlyConnectionOpen = false;
}

// drain
// Empties incoming buffer 

int WiFlySerial::drain() {
	char chDrain;
	unsigned long TimeOutTime = millis() + DEFAULT_WAIT_TIME;
	// DEBUG_LOG(3, "Waiting for signal");
	while ( _bWiFlyConnectionOpen && _uart->available() > 0 && millis() < TimeOutTime  ) {
		chDrain = _uart->read();
		DEBUG_LOG(3,  chDrain );
	}
	DEBUG_LOG(3, "Drained.");
    
	
}


// serveConnection
// Waits for a client to connect on the given port.
//
// Parameters:
// reconnectWaitTime  Duration to wait before verifying wlan and reconnecting if needed.
//
// returns            true on connection, false on internal failure.
//
bool WiFlySerial::serveConnection( const unsigned long reconnectWaitTime )
{
    char bufRequest[COMMAND_BUFFER_SIZE];
    int  iRequest;
    //DEBUG_LOG(3, "Serve Connection");
    iRequest = ScanForPattern( bufRequest, COMMAND_BUFFER_SIZE, WiFlyFixedPrompts[WIFLY_MSG_OPEN], true, reconnectWaitTime );
    
    if ( iRequest & PROMPT_EXPECTED_TOKEN_FOUND ) {
        //memset (bufRequest,'\0',COMMAND_BUFFER_SIZE);
        _bWiFlyInCommandMode = false;
        _bWiFlyConnectionOpen = true;
        return true;
    } 
    
    // TODO: decision on timeout/reconnection status
    
	return false;
}



// getMAC
// Returns WiFly MAC address
//
// Parameters: 
// bufMAC    buffer for MAC address
// buflen    length of buffer (should be at least 18 chars)
// Returns:  pointer to supplied buffer MAC address, or empty string on failure.
// Format expected: Mac Addr=xx:xx:xx:xx:xx:xx
char* WiFlySerial::getMAC(char* pbuf, int buflen) {
    return ExtractDetailIdx( STI_WIFLYDEVICE_GET_MAC, pbuf, buflen, STI_WIFLYDEVICE_GET_MAC_ADDR, STI_WIFLYDEVICE_RETURN ) ;
}

// getIP
// Returns WiFly IP address
//
// Parameters
// bufIP     buffer for IP address
// buflen    length of buffer (should be at least 18 chars for IPv4 and longer for IPv6)
// Returns:  pointer to supplied buffer with IP address, will be empty string on failure.
// 
char* WiFlySerial::getIP(char* pbuf, int buflen) {
    return ExtractDetailIdx( STI_WIFLYDEVICE_GET_IP_DETAILS, pbuf, buflen, STI_WIFLYDEVICE_GET_IP_IND, STI_WIFLYDEVICE_RETURN ) ;
}

// getNetMask
// Returns WiFly Netmask
//
// Parameters
// buf     buffer for Netmask 
// buflen    length of buffer (should be at least 18 chars for IPv4 and longer for IPv6)
// Returns:  pointer to supplied buffer with GW address, will be empty string on failure.
char* WiFlySerial::getNetMask(char* pbuf, int buflen) {
    return ExtractDetailIdx( STI_WIFLYDEVICE_GET_IP_DETAILS, pbuf, buflen, STI_WIFLYDEVICE_GET_NM_IND, STI_WIFLYDEVICE_RETURN ) ;
}


// getGateway
// Returns WiFly Gateway address
//
// Parameters
// bufGW     buffer for IP address
// buflen    length of buffer (should be at least 18 chars for IPv4 and longer for IPv6)
// Returns:  pointer to supplied buffer with GW address, will be empty string on failure.
char* WiFlySerial::getGateway(char* pbuf, int buflen) {
    return ExtractDetailIdx( STI_WIFLYDEVICE_GET_IP_DETAILS, pbuf, buflen, STI_WIFLYDEVICE_GET_GW_IND, STI_WIFLYDEVICE_RETURN ) ;
}


// getDNS
// Returns WiFly DNS address
//
// Parameters
// bufDNS    buffer for IP address
// buflen    length of buffer (should be at least 18 chars for IPv4 and longer for IPv6)
// Returns:  pointer to supplied buffer with DNS address, will be empty string on failure.
char* WiFlySerial::getDNS(char* pbuf, int buflen) {
    return ExtractDetailIdx( STI_WIFLYDEVICE_GET_DNS_DETAILS, pbuf, buflen, STI_WIFLYDEVICE_GET_DNS_IND, STI_WIFLYDEVICE_RETURN ) ;
    
}

// getSSID
// Returns current SSID
//
// Parameters
// bufSSID   buffer for SSID
// buflen    length of buffer (should be at least 18 chars for IPv4 and longer for IPv6)
// Returns:  pointer to supplied buffer with SSID, will be empty string on failure.
// 
char* WiFlySerial::getSSID(char* pbuf, int buflen) {
    return ExtractDetailIdx( STI_WIFLYDEVICE_GET_WLAN, pbuf, buflen, STI_WIFLYDEVICE_GET_WLAN_SSID_IND, STI_WIFLYDEVICE_RETURN ) ;
}

// getRSSI
// Returns current RSSI
//
// Parameters
// pbuf      buffer for RSSI
// buflen    length of buffer 
// Returns:  pointer to supplied buffer with RSSI, will be empty string on failure.
// 
char* WiFlySerial::getRSSI(char* pbuf, int buflen) {
    return ExtractDetailIdx( STI_WIFLYDEVICE_GET_RSSI, pbuf, buflen, STI_WIFLYDEVICE_GET_RSSI_IND, STI_WIFLYDEVICE_RETURN ) ;
}

// getBattery
// Returns current Battery voltage
//
// Parameters
// pbuf      buffer for voltage
// buflen    length of buffer 
// Returns:  pointer to supplied buffer with battery voltage, will be empty string on failure.
// 
char* WiFlySerial::getBattery(char* pbuf, int buflen) {
    return ExtractDetailIdx( STI_WIFLYDEVICE_GET_BATTERY, pbuf, buflen, STI_WIFLYDEVICE_GET_BATTERY_IND, STI_WIFLYDEVICE_RETURN ) ;
}

// getDeviceID
// Returns current DeviceID
//
// Parameters
// bufSSID   buffer for DeviceID
// buflen    length of buffer (should be at least 18 chars for IPv4 and longer for IPv6)
// Returns:  pointer to supplied buffer with DeviceID, will be empty string on failure.
// 
char* WiFlySerial::getDeviceID(char* pbuf, int buflen) {
    return ExtractDetailIdx( STI_WIFLYDEVICE_GET_WLAN, pbuf, buflen, STI_WIFLYDEVICE_GET_WLAN_DEV_IND, STI_WIFLYDEVICE_RETURN ) ;
}


// getTime
// Returns (in milliseconds) time since bootup or Unix epoch if NTP server updated.
//
// Parameters: 
// Returns:  unsigned long representing seconds since Unix Epoch or power-on.
// Format expected: RTC=tttttttt ms
unsigned long WiFlySerial::getTime() {
    char bufTimeSerial[COMMAND_BUFFER_SIZE];
    
    return ( (long)(ExtractDetailIdx( STI_WIFLYDEVICE_GET_TIME, bufTimeSerial, COMMAND_BUFFER_SIZE, STI_WIFLYDEVICE_GET_TIME_IND, STI_WIFLYDEVICE_RETURN ) ) + lUTC_Offset_seconds);
}

// TODO: add GetTimeStatus


// Function for setSyncProvider
time_t WiFlySerial::getSyncTime() {
    
    // wifly returns UTC time.
    time_t tCurrent = (time_t) getTime();
    // DebugPrint( "RESYNC TIME FROM WIFLY *****" );
    exitCommandMode();
    return tCurrent;
}


// isTCPConnected
// 
// Returns true if TCP Connected, false if not.
// Dependent on prior call to getDeviceStatus();
// WiFly 'show connection' command gives a result in hex 8XYZ as a string.
// bit 1
//
// Parameters
// None. 
// Returns true on success, false on failure.
// 
bool WiFlySerial::isTCPConnected() { 
    
    if ( (_fStatus & 0x8000) == 0 ) {
        
        getDeviceStatus();
    }
    
    
    return  (_fStatus &  0x01 > 0) ;
    
}

// isAssociated
// 
// Returns true if associated with a wifi SSID, false if not.
// Dependent on prior call to getDeviceStatus();
// WiFly 'show connection' command gives a result in hex 8XYZ as a string.
// bit 4
//
// Parameters
// None. 
// Returns true on success, false on failure.
// 
bool WiFlySerial::isAssociated() {
    
    if ( (_fStatus & 0x8000) == 0 ) {
        
        getDeviceStatus();
    }
    
    
    return ( _fStatus & 0x10 );
    
}

// isAuthenticated
// 
// Returns true if authenticated with a wifi SSID, false if not.
// Dependent on prior call to getDeviceStatus();
// WiFly 'show connection' command gives a result in hex 8XYZ as a string.
// bit 5
//
// Parameters
// None. 
// 
bool WiFlySerial::isAuthenticated() {
    
    if ( (_fStatus & 0x8000) == 0 ) {
        
        getDeviceStatus();
    }
    
    
    return ( _fStatus & 0x20) ;
    
}

// isDNSfound
// 
// Returns true if DNS is found, false if not.
// Dependent on prior call to getDeviceStatus();
// WiFly 'show connection' command gives a result in hex 8XYZ as a string.
// bit 6
//
// Parameters
// None. 
// 
bool WiFlySerial::isDNSfound() {
    
    if ( (_fStatus & 0x8000) == 0 ) {
        
        getDeviceStatus();
    }
    
    
    return ( _fStatus & 0x40) ;
    
}

// isDNScontacted
// 
// Returns true if DNS is contacted, false if not.
// Dependent on prior call to getDeviceStatus();
// WiFly 'show connection' command gives a result in hex 8XYZ as a string.
// bit 7
//
// Parameters
// None. 
// 
bool WiFlySerial::isDNScontacted() {
    if ( (_fStatus & 0x8000) == 0 ) {
        
        getDeviceStatus();
    }
    
    return ( (_fStatus & 0x80) > 0 ? true : false);
    
}

// getChannel
// 
// Returns wifi channel number .
// Dependent on prior call to getDeviceStatus();
// WiFly 'show connection' command gives a result in hex 8XYZ as a string.
// bits 9-
//
// Parameters
// None. 
// Returns true on success, false on failure.
// 
int WiFlySerial::getChannel() {
    
    if ( (_fStatus & 0x8000) == 0 ) {
        
        getDeviceStatus();
    }
    
    
    return ( (_fStatus >> 8 ) - 0x80);
    
}


// isInCommandMode
// 
// Returns true if WiFly in Command Mode false if not.
//
// Parameters
// None. 
// 
bool WiFlySerial::isInCommandMode() {
    
    return  _bWiFlyInCommandMode;
    
}



// isConnectionOpen
// 
// Returns true if an IP connection is open, false if not.
//
// Parameters
// None. 
// 
bool WiFlySerial::isConnectionOpen() {
    
    return  _bWiFlyConnectionOpen;
    
}



// isifUp
// Returns true if currently connected to Access Point.
//
// Parameters: None
// Returns:  IP netmask, or empty string on failure.
bool WiFlySerial::isifUp() {
    bool bReturn = false;
    char buf[COMMAND_BUFFER_SIZE];
    
    ExtractDetailIdx( STI_WIFLYDEVICE_GET_IP_DETAILS, buf, COMMAND_BUFFER_SIZE, STI_WIFLYDEVICE_GET_IP_UP_IND, STI_WIFLYDEVICE_RETURN ) ;
    bReturn = ( strcmp(buf,"UP" ) == 0 ? true : false ) ;
    
    return bReturn;
    
}


// ExtractLineFromBuffer
// Returns string extracted from provided buffer.
//
//
// Parameters: 
// idString         StringID of command to send 
// pBuffer          pointer to provided buffer
// bufsize          expected size of required buffer
// pStartPattern    pointer to null-terminated string identifying the start of desired line
// chTerminator     character to terminate desired line.

// Returns:  pointer to within provided buffer, with result or empty string on failure.
//char* WiFlySerial::ExtractLineFromBuffer(const int idString,  char* pBuffer, const int bufsize, const char* pStartPattern, const char* pTerminator) {
//    char* pStart;
//    char* pTerm;
//    bool bOk = false;
//    char szCommand[SMALL_COMMAND_BUFFER_SIZE];
//    char* pResponse = pBuffer;
//    
//    if ( !SendInquiry( GetBuffer_P(idString, szCommand, SMALL_COMMAND_BUFFER_SIZE) , pBuffer, bufsize) ) {
//        bOk = false;
//    } else {
//        pStart= strstr(pBuffer, pStartPattern);
//        if (pStart != NULL ) {
//            // Move pointer past the start pattern
//            pStart += strlen(pStartPattern);
//            pResponse = pStart;
//        }
//        pTerm = strstr( pStart, pTerminator);
//        if (pTerm == NULL ) {
//            bOk=false;
//        } else {
//            *(pTerm) = '\0';
//            bOk = true;
//        } // if end-of-line found
//        
//    }
//    if (!bOk) {
//        *(pResponse) = '\0';
//    }
//    return pResponse;
//}
//


// leave
// Disconnects from current WIFI lan
//
// Parameters: 
//
// Returns true on command success, false on failure.
bool WiFlySerial::leave() {
    
    bool bSendLeave = false;
    char szCmd[COMMAND_BUFFER_SIZE];
    
    bSendLeave = SendCommand(WiFlyDevice_string_table[STI_WIFLYDEVICE_LEAVE],  
                             WiFlyDevice_string_table[STI_WIFLYDEVICE_DEAUTH],
                             szCmd,  
                             COMMAND_BUFFER_SIZE, 
                             false );
    
    return bSendLeave;
    
}

bool WiFlySerial::setSSID( const char* pSSID){
    
    return issueSetting( STI_WIFLYDEVICE_SET_SSID, pSSID );
    
}


bool WiFlySerial::setPassphrase( const char* pPassphrase) {
    
    return issueSetting( STI_WIFLYDEVICE_SET_PASSPHRASE, pPassphrase );
    
}


// Sets NTP server address
bool WiFlySerial::setNTP(const char* pNTP) {
    
    return issueSetting( STI_WIFLYDEVICE_SET_NTP, pNTP );
    
}

// Sets frequency for NTP updates
bool WiFlySerial::setNTP_Update_Frequency(const char* pNTP_Update) {
    
    return issueSetting( STI_WIFLYDEVICE_SET_NTP_ENABLE, pNTP_Update );
}

// sets offset from UTC.
// Parameters:
// fltUTC_Offset_Hours  Number of hours from UTC  (e.g. PST = -8 )
// Setting is used during setTime().
// returns true always.
bool WiFlySerial::setNTP_UTC_Offset(float fltUTC_Offset_hours) {
	lUTC_Offset_seconds = (long) (fltUTC_Offset_hours * 60 * 60);
	return true;
}
// Sets WiFly DNS name
bool WiFlySerial::setDeviceID( const char* pDeviceID) {
    
    return issueSetting( STI_WIFLYDEVICE_SET_DEVICEID, pDeviceID );
    
}

// issueSetting
// Issues a WiFly setting command
// Parameters:
// IdxCommand    Index into string table of command
// pParam        null-terminated string of parameter. 
//               Command and parameter must be less than COMMAND_BUFFER_SIZE
//
// Returns - true on Command success, false on fail.
bool WiFlySerial::issueSetting( int idxCommand, const char* pParam) {
    
    char szReply[INDICATOR_BUFFER_SIZE];
    //  char szIndicator[INDICATOR_BUFFER_SIZE];
    char szBuffer[COMMAND_BUFFER_SIZE];
    
    strcpy(szBuffer, WiFlyDevice_string_table[idxCommand]); 
    strncat(szBuffer, pParam, COMMAND_BUFFER_SIZE - strlen(szBuffer) );
    
    return SendCommand( szBuffer, WiFlyFixedPrompts[WIFLY_MSG_AOK], szReply,  INDICATOR_BUFFER_SIZE, true );
}

// SetUseDCHP
// Sets DHCP to requested mode 

bool WiFlySerial::setDHCPMode(const int iDHCPMode) {
    char bufMode[10];
    
    itoa( iDHCPMode, bufMode, 10);
    
    return issueSetting( STI_WIFLYDEVICE_SET_DHCP, bufMode );
}

// SetIP
// Sets static IP address
// Parameters:
// pIP      null-terminated character string of the IP address e.g. '192.168.1.3'
bool WiFlySerial::setIP( const char* pIP) {
    
    return issueSetting( STI_WIFLYDEVICE_SET_IP, pIP );
}

// SetNetMask
// Sets static IP netmask
// Parameters:
// pNM      null-terminated character string of the netmask e.g. '255.255.255.0'
bool WiFlySerial::setNetMask( const char* pNM) {
    
    return issueSetting( STI_WIFLYDEVICE_SET_NETMASK, pNM );
}

// SetGateway
// Sets static Gateway address
// Parameters:
// pGW      null-terminated character string of the Gateway address e.g. '192.168.1.254'
bool WiFlySerial::setGateway( const char* pGW) {
    
    return issueSetting( STI_WIFLYDEVICE_SET_GATEWAY, pGW );
}

// SetDNS
// Sets static DNS address
// Parameters:
// pDNS      null-terminated character string of the DNS address e.g. '192.168.1.1'
bool WiFlySerial::setDNS( const char* pDNS) {
    
    return issueSetting( STI_WIFLYDEVICE_SET_DNS, pDNS );
}

// SetChannel
// Sets wifi Channel 
// Parameters:
// pChannel      null-terminated character string of the channel e.g. '6'
bool WiFlySerial::setChannel( const char* pChannel) {
    
    
    return issueSetting( STI_WIFLYDEVICE_SET_CHANNEL, pChannel );
}

// SetAuthMode
// Sets wifi Authentication mode 
// Parameters:
// iAuthMode      Authentication mode of type WIFLY_AUTH_XXXX
bool WiFlySerial::setAuthMode( int iAuthMode) {
    char bufMode[10];
    
    itoa( iAuthMode, bufMode, 10);
    return issueSetting( STI_WIFLYDEVICE_SET_WIFI_AUTH, bufMode );
}

// SetJoinMode
// Sets wifi Network Join mode 
// Parameters:
// iJoinMode     join mode of type WIFLY_JOIN_XXXXXX

bool WiFlySerial::setJoinMode( int iJoinMode) {
    char bufMode[10];
    
    itoa( iJoinMode, bufMode, 10);
    return issueSetting( STI_WIFLYDEVICE_SET_WIFI_JOIN, bufMode );
}

// SetLocalPort
// Sets Local port for listening 
// Parameters:
// _iLocalPort    Port to listen to.

bool WiFlySerial::setLocalPort( int iNewLocalPort) {
    char bufPort[10];
    
    itoa( iNewLocalPort, bufPort, 10);
    this->_iLocalPort = iNewLocalPort;
    return issueSetting( STI_WIFLYDEVICE_SET_LOCAL_PORT, bufPort );
}

// SetRemotePort
// Sets Remote port for connections 
// Parameters:
// _iRemotePort    Port to contact on remote server.

bool WiFlySerial::setRemotePort( int iNewRemotePort) {
    char bufPort[10];
    
    itoa( iNewRemotePort, bufPort, 10);
    this->_iRemotePort = iNewRemotePort;
    return issueSetting( STI_WIFLYDEVICE_SET_REMOTE_PORT, bufPort );
}

// SetBaudrate
// Sets baudrate
// Parameters:
// baudrate    Port to contact on remote server.

bool WiFlySerial::setBaudrate(const char * iNewBaudrate) {
    return issueSetting( STI_WIFLYDEVICE_SET_UART_BAUD, iNewBaudrate );
}


bool WiFlySerial::saveSetting(){
	char szReturn[SMALL_COMMAND_BUFFER_SIZE];
    return SendCommand(WiFlyDevice_string_table[STI_WIFLYDEVICE_SAVE] , WiFlyFixedPrompts[WIFLY_MSG_AOK], szReturn, SMALL_COMMAND_BUFFER_SIZE );
}


// join
// Parameters: None
// Joins a network with previously supplied setSSID and passphrase.
//
// returns true on success, false on failure
bool WiFlySerial::join() {
    
    char szSSID[COMMAND_BUFFER_SIZE];
    return join( getSSID( szSSID, COMMAND_BUFFER_SIZE ));

}

// join
// Parameters: None
// Joins a network with given SSID and previously-provided passphrase.
//
// returns true on success, false on failure.
// Todo: support spaces in passphrase.
bool WiFlySerial::join(char* pSSID) {
    
    bool bJoined = false;
    char szCmd[COMMAND_BUFFER_SIZE];
    setSSID(pSSID);
    strcpy(szCmd, WiFlyDevice_string_table[STI_WIFLYDEVICE_INDEX_JOIN]);
    strncat( szCmd, pSSID, COMMAND_BUFFER_SIZE - strlen(szCmd) );
    
    char bufResponse [RESPONSE_BUFFER_SIZE];
    
    bJoined = SendCommand(szCmd,
                          WiFlyDevice_string_table[STI_WIFLYDEVICE_INDEX_ASSOCIATED],
                          bufResponse, 
                          RESPONSE_BUFFER_SIZE, 
                          true, 
                          JOIN_WAIT_TIME, false, false);
    
    return bJoined;
    
}

 void WiFlySerial::flush(){
    _uart->flush();
}

void WiFlySerial::write(uint8_t byte){
    _uart->write(byte);
}


 uint8_t WiFlySerial::read(){
    return _uart->read();
}

 int WiFlySerial::available(){
    return _uart->available();
}

 int WiFlySerial::peek(){
    return _uart->peek();
}

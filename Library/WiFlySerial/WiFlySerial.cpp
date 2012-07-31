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

#include "WiFlySerial/WiFlySerial.h"


WiFlySerial::WiFlySerial() {

  // Set initial values for flags.
  // On Maple startup, WiFly state not known.
  bWiFlyInCommandMode = false;
  bWiFlyConnectionOpen = false;
  fStatus = WIFLY_STATUS_OFFLINE ;
  strcpy(szWiFlyPrompt, WiFlyFixedPrompts[WIFLY_MSG_PROMPT2] );  // ">"

  iLocalPort = WIFLY_DEFAULT_LOCAL_PORT;
  iRemotePort = WIFLY_DEFAULT_REMOTE_PORT;

  // default is UTC timezone
  lUTC_Offset_seconds = 0;

    // ensure a default sink.
  //pDebugChannel = NULL;

  pControl = WiFlyFixedPrompts[WIFLY_MSG_CLOSE];


}


// begin
// Initializes WiFly interface and starts communication with WiFly device.
//
// Parameters: none.
// Returns: true on initialize success, false on failure.
bool WiFlySerial::begin(SpiUartDevice * TheSpi) {

  bool bStart = false;
  char szCmd[SMALL_COMMAND_BUFFER_SIZE];
  char szResponse[COMMAND_BUFFER_SIZE];
//  char szIndicator[INDICATOR_BUFFER_SIZE];

    //// TODO: set default uart transmission speed to same as WiFly default speed.
  //uart.begin(WIFLY_DEFAULT_BAUD_RATE);


    uart = TheSpi;

  //Device may or may not be:
  // awake / asleep
  // net-connected / connection lost
  // IP assigned / no IP
  // in command mode / data mode
  // in known state / confused

  // Start by setting command prompt.

  bWiFlyInCommandMode = false;
  StartCommandMode(szCmd, SMALL_COMMAND_BUFFER_SIZE);

  // // turn off echo
  //   // set baud rate
  //   bStart = SendCommand( GetBuffer_P(STI_WIFLYDEVICE_SET_UART_MODE, szCmd, SMALL_COMMAND_BUFFER_SIZE),
  //                          WiFlyFixedPrompts[WIFLY_MSG_AOK],
  //                          szResponse,
  //                          COMMAND_BUFFER_SIZE );
  //   bStart = SendCommand( GetBuffer_P(STI_WIFLYDEVICE_SET_UART_BAUD, szCmd, SMALL_COMMAND_BUFFER_SIZE),
  //                          WiFlyFixedPrompts[WIFLY_MSG_AOK],
  //                         szResponse,
  //                         COMMAND_BUFFER_SIZE );
  //   GetCmdPrompt();
  //   //DebugPrint("GotPrompt:");
  //   //DebugPrint(szWiFlyPrompt);
  //
  //   getDeviceStatus();
  //
  //   // try, then try again after reboot.
  //   if (strlen(szWiFlyPrompt) < 1 ) {
  //     // got a problem
  //     DebugPrint(GetBuffer_P(STI_WIFLYDEVICE_ERR_REBOOT, szCmd, SMALL_COMMAND_BUFFER_SIZE));
  //     reboot();
  //
  //     delay(WIFLY_RESTART_WAIT_TIME);
  //     // try again
  //
  //     GetCmdPrompt();
  //     if (strlen(szWiFlyPrompt) < 1 ) {
  //       DebugPrint(GetBuffer_P(STI_WIFLYDEVICE_ERR_START_FAIL, szCmd, SMALL_COMMAND_BUFFER_SIZE));
  //       bStart = false;
  //     }
  //   }

  return bStart;
}

bool WiFlySerial::StartCommandMode(char* pBuffer, const int bufSize) {
  byte iPromptResult = 0;
  char* responseBuffer;
  bool bWaiting = true;
  int nTries = 0;


  // if (pBuffer == NULL) {
  //       responseBuffer = (char*) malloc(bufSize); // defaults to COMMAND_BUFFER_SIZE
  //   } else {
  //     responseBuffer = pBuffer;
  //   }

   responseBuffer = pBuffer;
  unsigned long TimeOutTime = millis() + ATTN_WAIT_TIME;
  // check if actually in command mode:
  while  (!bWiFlyInCommandMode || bWaiting ) {

       // if not effectively in command mode, try $$$
       if ( !bWiFlyInCommandMode) {
        //Read everything
         uart->flush();
         // Send $$$ , wait a moment, look for CMD
         delay(COMMAND_MODE_GUARD_TIME );

         uart->print( GetBuffer_P(STI_WIFLYDEVICE_ATTN, responseBuffer, bufSize) ) ;

         delay(COMMAND_MODE_GUARD_TIME  );
         //
         // if (nTries >= 2)  {
         //    uart << "\r";
         //    uart.flush();
         //  }

         // expect CMD without a cr
         // WiFlyFixedPrompts[WIFLY_MSG_CMD]
         iPromptResult = ScanForPattern( responseBuffer, bufSize, "CMD", true, ATTN_WAIT_TIME);

         if ( iPromptResult & ( PROMPT_EXPECTED_TOKEN_FOUND | PROMPT_READY |PROMPT_CMD_MODE |PROMPT_CMD_ERR ) ) {
            bWiFlyInCommandMode = true;
            bWaiting = false;
        } else {
            bWiFlyInCommandMode = false;
         }      // if one of several indicators of command-mode received.

       } else {

        // think we are in a command-mode - try a cr, then add a version command to get through.
       // send a ver + cr, should see a prompt.
          if (nTries > 2)  {
            uart->println(GetBuffer_P(STI_WIFLYDEVICE_VER, responseBuffer, bufSize));
            // DebugPrint("ver=");
            // DebugPrint(responseBuffer);
          }

          // DebugPrint("***scm:InCommandMode***");

          // bring in a cr-terminated line
          // uart.flush(); //Why?
          // wait for up to time limit for a cr to flow by
          iPromptResult = ScanForPattern( responseBuffer, bufSize, szWiFlyPrompt, false);
          // could have timed out, or have *READY*, CMD or have a nice CR.

         if ( iPromptResult & ( PROMPT_EXPECTED_TOKEN_FOUND | PROMPT_AOK | PROMPT_READY |PROMPT_CMD_MODE |PROMPT_CMD_ERR ) ) {
               bWiFlyInCommandMode = true;
               bWaiting = false;
         } else {
               bWiFlyInCommandMode = false;
         }      // if one of several indicators of command-mode received.

       } //  else in in command command mode

      if (  millis() >= TimeOutTime) {
        bWaiting = false;
      }
      nTries++;
  } // while trying to get into command mode

  // // clean up as needed
  //   if (pBuffer == NULL) {
  //     free (responseBuffer);
  //   }
  return bWiFlyInCommandMode;
}

// GetBuffer_P
// Returns pointer to a supplied Buffer, from PROGMEM based on StringIndex provided.
// based on example from http://arduino.cc/en/Reference/PROGMEM
char* WiFlySerial::GetBuffer_P(const int StringIndex, char* pBuffer, int bufSize) {

  memset(pBuffer, '\0', bufSize);
  strncpy(pBuffer, (char*) &(WiFlyDevice_string_table[StringIndex]), bufSize);

  return pBuffer;

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
  int  iPromptIndex = 0;
  bool bWaiting = true;
  bool bReceivedCR = false;

 WiFlyFixedPrompts[WIFLY_MSG_EXPECTED] = (char*) pExpectedPrompt; //??
 WiFlyFixedPrompts[WIFLY_MSG_PROMPT] = (char*) szWiFlyPrompt;   //??
 char* pFixedCurrent[N_PROMPTS];


 for (int i=0; i < N_PROMPTS; i++) {
    pFixedCurrent[i] = WiFlyFixedPrompts[i];
 }

  memset (responseBuffer, '\0', bufsize);
  unsigned long TimeAtStart = millis()  ;  // capture current time

  while ( bWaiting ) {
    if ( uart->available() > 0 ) {
        chResponse = uart->read();
        DEBUG_LOG(3, chResponse);

        if ( bCollecting ) {
            responseBuffer[bufpos] = chResponse;
            if ( ++bufpos == bufsize ) {
                bufpos = 0;
            } // if buffer wrapped
        } // if capturing

        // Going through all possible Responds
        for (int iFixedPrompt = 0; iFixedPrompt < N_PROMPTS; iFixedPrompt++ ) {
            if ( chResponse == *pFixedCurrent[iFixedPrompt] ) {
            // deal with 'open' and 'scan' version-prompt appearing BEFORE result; ignore it
                if ( (!bPromptAfterResult) && (iFixedPrompt == WIFLY_MSG_PROMPT || iFixedPrompt == WIFLY_MSG_PROMPT2) /* standard version-prompt */  ) {
                    bWaiting = true;
                    iPromptFound |= PROMPT_READY;
                }
                else {
                    bWaiting = ( *(++pFixedCurrent[iFixedPrompt]) == '\0' ? false : true ) ; // done when end-of-string encountered.

                    if (!bWaiting) {
                        iPromptFound |= WiFlyFixedFlags[iFixedPrompt];  // if a prompt found then grab its flag.
                    }
                } // handle prompt-BEFORE-result case
            }
            else {
                pFixedCurrent[iFixedPrompt] = WiFlyFixedPrompts[iFixedPrompt];  // not next char expected; reset to beginning of string.
            } // if tracking expected response
        }
       // If the *OPEN* signal caught then a connection was opened.
       if (iPromptFound & (PROMPT_OPEN | PROMPT_OPEN_ALREADY)  ) {
         bWiFlyConnectionOpen = true;
         bWiFlyInCommandMode = false;
         iPromptFound &= (!WiFlyFixedFlags[WIFLY_MSG_CLOSE]);  // clear prior close
       }

       // If the *CLOS* signal caught then a connection was closed
       // and we dropped into command mode
       if (iPromptFound & PROMPT_CLOSE ) {
         bWiFlyConnectionOpen = false;
         bWiFlyInCommandMode = true;
         iPromptFound &= (!WiFlyFixedFlags[WIFLY_MSG_OPEN]);  // clear prior open
       }

    } // if anything in uart

    // did we time-out?
    if ( (millis() - TimeAtStart) >= WaitTime) {
        bWaiting = false;
    }
 }  // while waiting for a line
    // could capture and compare with known prompt
 if ( bCollecting ) {
    responseBuffer[bufpos]='\0';
 }
  return (int) iPromptFound;

} // ScanForPattern

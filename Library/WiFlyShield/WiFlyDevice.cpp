

#define DEBUG_LEVEL 0

#include "Debug.h"
#include <WiFlyShield/WiFlyDevice.h>

// Preinstantiate required objects
//SpiUartDevice *SpiSerial;
//WiFlyDevice WiFly(SpiSerial);
boolean WiFlyDevice::findInResponse(const char *toMatch,
                                    unsigned int timeOut = 1000) {
  /*

   */

  // TODO: Change 'sendCommand' to use 'findInResponse' and have timeouts,
  //       and then use 'sendCommand' in routines that call 'findInResponse'?

  // TODO: Don't reset timer after successful character read? Or have two
  //       types of timeout?

  int byteRead;

  unsigned long timeOutTarget; // in milliseconds

  DEBUG_LOG(1, "Entered findInResponse");
  DEBUG_LOG(2, "Want to match:");
  DEBUG_LOG(2, toMatch);
  DEBUG_LOG(3, "Found:");

  for (unsigned int offset = 0; offset < strlen(toMatch); offset++) {

    // Reset after successful character read
    timeOutTarget = millis() + timeOut; // Doesn't handle timer wrapping

    while (!uart->available()) {
      // Wait, with optional time out.
      if (timeOut > 0) {
        if (millis() > timeOutTarget) {
          return false;
        }
      }
      delay(1); // This seems to improve reliability slightly
    }

    // We read this separately from the conditional statement so we can
    // log the character read when debugging.
    byteRead = uart->read();

    delay(1); // Removing logging may affect timing slightly

    DEBUG_LOG(5, "Offset:");
    DEBUG_LOG(5, offset);
    DEBUG_LOG(3, (char) byteRead);
    DEBUG_LOG(4, byteRead);

    if (byteRead != toMatch[offset]) {
      offset = 0;
      // Ignore character read if it's not a match for the start of the string
      if (byteRead != toMatch[offset]) {
        offset = -1;
      }
      continue;
    }
  }
  DEBUG_LOG(2, "Response found");

  return true;
}



WiFlyDevice::WiFlyDevice() {
  /*

    Note: Supplied UART should/need not have been initialised first.

   */

  bDifferentUart = 0;

  // The WiFly requires the server port to be set between the `reboot`
  // and `join` commands so we go for a "useful" default first.
  serverPort = DEFAULT_SERVER_PORT;
  serverConnectionActive = false;
}

void WiFlyDevice::begin(SpiUartDevice * theUart) {
    DEBUG_LOG(1, "Entered WiFlyDevice::begin()");
    uart = theUart;
    softwarereboot(); // Reboot to get device into known state

}

// void WiFlyDevice::begin() {
//   /*
//    */
//
//
//   // if (!bDifferentUart) SPIuart.begin();
//  // reboot(); // Reboot to get device into known state
//   //requireFlowControl();
//   //setConfiguration(adhocMode);
// }

// TODO: Create a `begin()` that allows IP etc to be supplied.

bool WiFlyDevice::softwareReboot(bool isAfterBoot = true) {
  /*

   */

  DEBUG_LOG(1, "Entered softwareReboot");

  for (int retryCount = 0;
       retryCount < SOFTWARE_REBOOT_RETRY_ATTEMPTS;
       retryCount++) {

    // TODO: Have the post-boot delay here rather than in enterCommandMode()?

    if (!enterCommandMode(isAfterBoot)) {
      return false; // If the included retries have failed we give up
    }

    uart->println("reboot");

    // For some reason the full "*Reboot*" message doesn't always
    // seem to be received so we look for the later "*READY*" message instead.

    // TODO: Extract information from boot? e.g. version and MAC address

    if (findInResponse("*READY*", 2000)) {
           return true;
         }
  }

  return false;
}

bool WiFlyDevice::enterCommandMode(bool isAfterBoot) {
  /*

   */

  DEBUG_LOG(1, "Entered enterCommandMode");

  // Note: We used to first try to exit command mode in case we were
  //       already in it. Doing this actually seems to be less
  //       reliable so instead we now just ignore the errors from
  //       sending the "$$$" in command mode.

  for (int retryCount = 0;
       retryCount < COMMAND_MODE_ENTER_RETRY_ATTEMPTS;
       retryCount++) {

    // At first I tried automatically performing the
    // wait-send-wait-send-send process twice before checking if it
    // succeeded. But I removed the automatic retransmission even
    // though it makes things  marginally less reliable because it speeds
    // up the (hopefully) more common case of it working after one
    // transmission. We also now have automatic-retries for the whole
    // process now so it's less important anyway.

    if (isAfterBoot) {
      delay(1000); // This delay is so characters aren't missed after a reboot.
    }

    delay(COMMAND_MODE_GUARD_TIME);

    uart->print("$$$");

    delay(COMMAND_MODE_GUARD_TIME);

    // We could already be in command mode or not.
    // We could also have a half entered command.
    // If we have a half entered command the "$$$" we've just added
    // could succeed or it could trigger an error--there's a small
    // chance it could also screw something up (by being a valid
    // argument) but hopefully it's not a general issue.  Sending
    // these two newlines is intended to clear any partial commands if
    // we're in command mode and in either case trigger the display of
    // the version prompt (not that we actually check for it at the moment
    // (anymore)).

    // TODO: Determine if we need less boilerplate here.

    // CHANGE! Didn't understand the 2 newlines…
    uart->flush();


    // TODO: Add flush with timeout here?

    // This is used to determine whether command mode has been entered
    // successfully.
    // TODO: Find alternate approach or only use this method after a (re)boot?
    uart->println("ver");

    if (findInResponse("\r\nWiFly Ver", 1000)) {
      // TODO: Flush or leave remainder of output?
      return true;
    }
  }
  return false;
}




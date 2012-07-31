#ifndef __WIFLY_DEVICE_H__
#define __WIFLY_DEVICE_H__

#include "Configuration.h"
#include "libmaple.h"
#include "SpiUart.h"

#define DEFAULT_SERVER_PORT 80

#define SOFTWARE_REBOOT_RETRY_ATTEMPTS 5

#define COMMAND_MODE_ENTER_RETRY_ATTEMPTS 5

#define COMMAND_MODE_GUARD_TIME 250 // in milliseconds

class WiFlyDevice {
  public:
    WiFlyDevice();

    // void setUart(Stream* newUart);
    void begin(SpiUartDevice * theUart);
    //void begin(bool adhocMode);
    //     bool createAdHocNetwork(const char *ssid);
    //
    //     bool join(const char *ssid);
    //     bool join(const char *ssid, const char *passphrase,
    //                  bool isWPA = true);
    //
    //     bool configure(byte option, unsigned long value);
    //
    //     long getTime();
    //
    //     const char * ip();

  private:
    //SpiUartDevice * SPIuart;
    SpiUartDevice * uart;
    bool bDifferentUart;
    //    // Okay, this really sucks, but at the moment it works.
    //    // The problem is that we have to keep track of an active server connection
    //    // but AFAICT due to the way the WebClient example is written
    //    // we can't store a useful reference in the server instance
    //    // to an active client instance because the client object gets copied
    //    // when it's returned from Server.available(). This means that
    //    // the state changes in the client object's Client.stop() method
    //    // never get propagated to the Server's stored active client.
    //    // Blah, blah, hand-wavy singleton mention. Trying to store the reference
    //    // to the active client connection here runs into apparent circular
    //    // reference issues with header includes. So in an effort to get this out
    //    // the door we just share whether or not the current "active client"
    //    // that the server has a stored reference is actually active or not.
    //    // (Yeah, nice.)
    //    // TODO: Handle this better.
    bool serverConnectionActive;
    //
    uint32 serverPort;
    //
    //    // TODO: Should these be part of a different class?
    //    // TODO: Should all methods that need to be in command mode ensure
    //    //       they are first?
    //    void attemptSwitchToCommandMode();
    //    void switchToCommandMode();
    //    void reboot();
    //    void requireFlowControl();
    //    void setConfiguration(bool adhocMode);
    //    void setAdhocParams();
    //    bool sendCommand(const char *command,
    //                        bool isMultipartCommand, // Has default value
    //                        const char *expectedResponse); // Has default value
    //    bool sendCommand(const __FlashStringHelper *command,
    //                        bool isMultipartCommand, // Has default value
    //                        const char *expectedResponse); // Has default value
    //    void waitForResponse(const char *toMatch);
    //    void skipRemainderOfResponse();
    //    bool responseMatched(const char *toMatch);
    //
    //    bool findInResponse(const char *toMatch, unsigned int timeOut);
    bool enterCommandMode(bool isAfterBoot = false);
    bool softwareReboot(bool isAfterBoot);
    //    bool hardwareReboot();
    //
    //    friend class WiFlyClient;
    //    friend class WiFlyServer;
};

#endif

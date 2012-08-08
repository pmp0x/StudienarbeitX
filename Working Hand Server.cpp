/*
#include <FatStructs.h>
#include <Sd2Card.h>
#include <Sd2PinMap.h>
#include <SdFatmainpage.h>
#include <SdFatUtil.h>
#include <SdInfo.h>
*/

#include <libmaple.h>
#include <wirish.h>
#include <WiFlySerial/SpiUart.h>
#include <WiFlySerial/WiFlySerial.h>
#include "WiFlySerial/WFSEthernet.h"




HardwareSPI spi(1);
SpiUartDevice SpiSerial;
WiFlySerial WiFly;
WFSEthernet Ethernet;
WFSEthernetServer myServer(80);

#define REQUEST_BUFFER_SIZE 120
#define HEADER_BUFFER_SIZE 150 
#define BODY_BUFFER_SIZE 100

char bufRequest[REQUEST_BUFFER_SIZE];
char bufHeader[HEADER_BUFFER_SIZE];
char bufBody[BODY_BUFFER_SIZE];

// Make Response Body
// Based on GET request string, generate a response.


void setup()
{
	
	spi.begin(SPI_562_500KHZ, MSBFIRST, 0);
    SpiSerial.begin(&spi);
    while (!SerialUSB.available());
    SerialUSB.println("Foo");
    SpiSerial.begin(&spi, 9600);
	delay(100);
    WiFly.begin(&SpiSerial);
    

    Ethernet.begin(&WiFly);
    
    myServer.begin(&Ethernet);
   
char bufRequest[200];
   memset(bufRequest, '\0', 200);
//    // get MAC
//    SerialUSB.println("MAC:");
//    SerialUSB.println( WiFly.getMAC(bufRequest, REQUEST_BUFFER_SIZE) );
    // is connected ?
    
    // WiFly.setDebugChannel( (Print*) &Serial);
//    
//    WiFly.setAuthMode( WIFLY_AUTH_WPA2_PSK);
//    WiFly.setJoinMode(  WIFLY_JOIN_AUTO );
//    WiFly.setDHCPMode( WIFLY_DHCP_ON );
//    
//    
//    // if not connected restart link
//    WiFly.getDeviceStatus();
//    if (! WiFly.isifUp() ) {
//        Serial << "Leave:" <<  ssid << WiFly.leave() << endl;
//        // join
//        if (WiFly.setSSID(ssid) ) {    
//            Serial << "SSID Set :" << ssid << endl;
//        }
//        if (WiFly.setPassphrase(passphrase)) {
//            Serial << "Passphrase Set :" << endl;
//        }
//        Serial << "Joining... :"<< ssid << endl;
//        
//        if ( WiFly.join() ) {
//            Serial << F("Joined ") << ssid << F(" successfully.") << endl;
//            WiFly.setNTP( ntp_server ); // use your favorite NTP server
//        } else {
//            Serial << F("Join to ") << ssid << F(" failed.") << endl;
//        }
//    } // if not connected
    
    SerialUSB.print("IP: ");
    SerialUSB.println( WiFly.getIP(bufRequest, REQUEST_BUFFER_SIZE)) ;
//    SerialUSB.print("Netmask: "); 
//    SerialUSB.println(WiFly.getNetMask(bufRequest, REQUEST_BUFFER_SIZE) );
//    SerialUSB.print("Gateway: ");
//    SerialUSB.println( WiFly.getGateway(bufRequest, REQUEST_BUFFER_SIZE) ); 
//
//    SerialUSB.print(("DNS: ")); 
//    SerialUSB.println(WiFly.getDNS(bufRequest, REQUEST_BUFFER_SIZE) );
//    SerialUSB.print("WiFly Sensors: ");
//    SerialUSB.println( WiFly.SendCommand("show q 0x177 ",">", bufBody, BODY_BUFFER_SIZE));
    
//    SerialUSB.println("Bool things");
//    SerialUSB.println(WiFly.getDeviceStatus() );
//    SerialUSB.println(WiFly.isAssociated()    );
//    SerialUSB.println(WiFly.isTCPConnected()  );
//    SerialUSB.println(WiFly.isAuthenticated() );
//    SerialUSB.println(WiFly.isifUp()          );
//    SerialUSB.println("Channel");
//    SerialUSB.println(WiFly.getChannel()      );
//    SerialUSB.println(WiFly.isDNSfound()     );
//    SerialUSB.println(WiFly.isDNScontacted()  );
//    SerialUSB.println(WiFly.isInCommandMode() );
//    SerialUSB.println(WiFly.isConnectionOpen());
//    
//                                        
//    SerialUSB.print(("WiFly Temp: ")); 
//    SerialUSB.println( WiFly.SendCommand("show q t ",">", bufBody, BODY_BUFFER_SIZE));
//
//    WiFly.SendCommand("set comm remote 0",">", bufBody, BODY_BUFFER_SIZE);



	WiFly.exitCommandMode();
    WiFly.flush();
	delay(800);
    WiFly.flush();    

    SerialUSB.println("WiFly now listening for commands.  Type 'exit' to listen for wifi traffic.  $$$ (no CR) for command-mode.");
                                        
    
    // clear out prior requests.
 
    
}


void loop()
{

    // if not connected restart link
//    while (! WiFly.isAssociated() ) {
//        SerialUSB.println("Not associated - Rejoining");
//    } // while not connected
    // Eigentlich noch die connection überprüfen
    
	WFSEthernetClient myClient = myServer.available();
    // iRequest = wifi.serveConnection();

    if (myClient.connected() ){
   
        SerialUSB.println("Connected ");
               
        
        myClient.stop();
        
	}
    
    
//    while(SpiSerial.available() > 0 ){
//        SerialUSB.write(SpiSerial.read());
//    }
//    while(SerialUSB.available()){
//        SpiSerial.write(SerialUSB.read());
//    }
    
    
}

// Standard libmaple init() and main.
//
// The init() part makes sure your board gets set up correctly. It's
// best to leave that alone unless you know what you're doing. main()
// is the usual "call setup(), then loop() forever", but of course can
// be whatever you want.

__attribute__((constructor)) void premain() {
    init();
}

int main(void) {
    setup();

    while (true) {
        loop();
    }

    return 0;
}
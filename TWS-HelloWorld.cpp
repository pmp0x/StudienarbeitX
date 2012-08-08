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

#include "TinyWebServer.h"


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



bool index_handler(TinyWebServer& web_server);

TinyWebServer::PathHandler handlers[] = {
    // Work around Arduino's IDE preprocessor bug in handling /* inside
    // strings.
    //
    // `put_handler' is defined in TinyWebServer
    {"/", TinyWebServer::GET, &index_handler },
    {NULL},
};

const char* headers[] = {
    "Content-Length",
    NULL
};




bool index_handler(TinyWebServer& web_server) {

    return true;
}

TinyWebServer web(handlers, headers);
void setup()
{
	
	spi.begin(SPI_562_500KHZ, MSBFIRST, 0);
    SpiSerial.begin(&spi);
    while (!SerialUSB.available());
    SerialUSB.println("Foo");
    SpiSerial.begin(&spi, 9600);
	delay(100);
    WiFly.begin(&SpiSerial);
    
    
    //Ethernet.begin(&WiFly);
    
    //myServer.begin(&Ethernet);
    
    char bufRequest[200];
    memset(bufRequest, '\0', 200);
    SerialUSB.print("IP: ");
    SerialUSB.println( WiFly.getIP(bufRequest, REQUEST_BUFFER_SIZE)) ;
       
	WiFly.exitCommandMode();
    WiFly.flush();
	delay(800);
    WiFly.flush();    
    
    Ethernet.begin(&WiFly);
    SerialUSB.println("WiFly now listening for commands.  Type 'exit' to listen for wifi traffic.  $$$ (no CR) for command-mode.");
    
    web.begin(&Ethernet);
   
    
    
}


void loop()
{
    
     web.process();
    
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
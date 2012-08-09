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
#include <HardwareSPI.h>
#include <stdint.h>
#include <string.h>

#include <WiFlySerial/SpiUart.h>
#include "WiFlySerial/WiFlySerial.h"
HardwareSPI spi(1);
SpiUartDevice SpiSerial;
WiFlySerial wifly;


void setup()
{
    SerialUSB.begin();
    spi.begin(SPI_18MHZ, MSBFIRST, 0);
    while (!SerialUSB.available());
    SerialUSB.println("Foo");    

    
    SpiSerial.begin(&spi, 9600);
    SpiSerial.writeRegister(SPR, 'h');
    // Need some time to transfer…
    //    delay(60);
    SerialUSB.println (SpiSerial.readRegister(SPR) );

  
	    delay(100);
    
    wifly.begin(&SpiSerial);
    delay(100);
    wifly.setBaudrate("921600");
    SpiSerial.begin(&spi, 921600);
    delay(100);
    //wifly.saveSetting();
    
    
    
    //    for(int i=0; i<16; i++){
    //            SerialUSB.print("Reg ");
    //               SerialUSB.println(i, HEX);
    //               SerialUSB.println(SpiSerial.readRegister(i<<3), BIN);
    //             }
    //            spi.write(0xff);
    SerialUSB.println("got here");
    
}

void loop()
{
    while(SpiSerial.available() > 0 ){
        SerialUSB.print("(");
		SerialUSB.print(SpiSerial.available() );
        SerialUSB.print(")");   
        SerialUSB.write(SpiSerial.read());
    }
    while(SerialUSB.available()){
        SpiSerial.write(SerialUSB.read());
    }
//    while (SpiSerial.available()) {
//        SerialUSB.print( (char)SpiSerial.read() );
//    }

    // uint8 buf[64];
    //    SerialUSB.println(SpiSerial.available());
    //    if(SpiSerial.available() > 1){
    //        SpiSerial.bulk_read(buf, SpiSerial.available());
    //        int i=0;
    //        SerialUSB.println("Got here");
    //        while(buf != '\0' || i < 64){
    //            SerialUSB.write(buf[i++]);
    //        }
    //
    //    }
    // delay(10);
    //
    
//    while( SpiSerial.available() > 0 ){
////        SerialUSB.print("(");
////        SerialUSB.print(SpiSerial.available() );
////        SerialUSB.print(")");       
//        SerialUSB.print((char)SpiSerial.read());
//
//    }
//        delay(10);
//    while( SerialUSB.available() ){
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
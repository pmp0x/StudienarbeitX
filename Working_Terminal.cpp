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

HardwareSPI spi(1);
SpiUartDevice SpiSerial;


char *fiforead(){
    uint32 const rdbl = SpiSerial.available();
    char buf[rdbl];
    uint32 rxved = 0;
    SerialUSB.println("RDBL");
    SerialUSB.println(rdbl);
    while(rdbl > 0 && rxved < rdbl) {
        // eigentlich muss das ja nur einmal gesendet werden… und was passiert, wenn zwischendurch etwas weiteres über UART kommt?!
        buf[rxved++] = char(spi.transfer(RHR));

    }
    return buf;
}

void read(){
    uint32 length;
    length = SpiSerial.available();

    spi.write(RHR | 0x80);


}


void setup()
{
    SerialUSB.begin();
    spi.begin(SPI_562_500KHZ, MSBFIRST, 0);

    while (!SerialUSB.available());
    SerialUSB.println("Foo");
    SpiSerial.begin(&spi, 9600);
     delay(100);

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

    while(SpiSerial.available() > 0 ){
        SerialUSB.write(SpiSerial.read());
    }
    while(SerialUSB.available()){
        SpiSerial.write(SerialUSB.read());
    }




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
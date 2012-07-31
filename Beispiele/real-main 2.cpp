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

#include <SpiUart.h>

HardwareSPI spi(1);
SpiUartDevice SpiSerial;

uint32 fifowrite(char * str_out){
    spi.write(THR);
    spi.write(str_out, strlen(str_out));

}

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
    spi.begin(SPI_281_250KHZ, MSBFIRST, 0);

    while (!SerialUSB.available());
    SerialUSB.println("Foo");
    SpiSerial.begin(&spi, 9600);

    const char TEST_CHARACTER = 'H';
SerialUSB.println(TEST_CHARACTER);
SerialUSB.println((char)TEST_CHARACTER);
      SpiSerial.writeRegister(SPR, TEST_CHARACTER);
      // Need some time to transfer…
      delay(60);
      SerialUSB.println( char(SpiSerial.readRegister(SPR)) );

      SerialUSB.println( SpiSerial.readRegister(LSR), BIN );
    const int damn = 32;
    char buff[damn];



    SerialUSB.println(" * Use $$$ (with no line ending) to enter WiFly command mode. (\"CMD\")");
    SerialUSB.println(" * Then send each command followed by a carriage return.");
    SerialUSB.println();

    SerialUSB.println("Waiting for input.");
    SerialUSB.println();


//     if (SpiSerial.readRegister(RXLVL) == 1 ){
//         SerialUSB.println("|Data|");
//         SerialUSB.write(SpiSerial.readRegister(RHR));
//     }
//     SerialUSB.println("RXLVL");
//     SerialUSB.println(SpiSerial.readRegister(RXLVL));
//
//
//
//                delay(200);
//
//              buff[0] = THR;
//                 buff[1] = '$';
//                 buff[2] = '$';
//                 buff[3] = '$';
// SerialUSB.println("foo");

//uint32 fifowrite(char * str_out){
}

void loop()
{

 SerialUSB.println("RXLVL");
    SerialUSB.println((uint8)SpiSerial.readRegister(RXLVL));
 SerialUSB.println("LSR");
    SerialUSB.println((uint8)SpiSerial.readRegister(LSR));
    // if (SpiSerial.readRegister(RXLVL) > 0 ){
    //        SerialUSB.println("RXLVL");
    //        SerialUSB.println(SpiSerial.readRegister(RXLVL));
    //        SerialUSB.println("LSR");
    //        SerialUSB.println(SpiSerial.readRegister(LSR));
    //        SerialUSB.println("|Data|");
    //        SerialUSB.write(SpiSerial.readRegister(RHR));
    //    }
    delay(300);
    // while(SpiSerial.available() > 0) {
    //        SerialUSB.write(char(SpiSerial.read()));
    //        SerialUSB.write(",");
    //    }
    //
       // if(SerialUSB.available()) { // Outgoing data
       //            SpiSerial.write( char(SerialUSB.read()));
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
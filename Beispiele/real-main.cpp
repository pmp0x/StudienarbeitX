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


void setup()
{
    SerialUSB.begin();
    spi.begin(SPI_281_250KHZ, MSBFIRST, 0);

    while (!SerialUSB.available());
    SerialUSB.println("Foo");
    SpiSerial.begin(&spi, 9600);

    SerialUSB.println(" * Use $$$ (with no line ending) to enter WiFly command mode. (\"CMD\")");
    SerialUSB.println(" * Then send each command followed by a carriage return.");
    SerialUSB.println();

    SerialUSB.println("Waiting for input.");
    SerialUSB.println();

    SerialUSB.println("LSR");
    SerialUSB.println(  SpiSerial.readRegister(LSR) );
    SerialUSB.println("RHR");
    SerialUSB.println(  SpiSerial.readRegister(RHR) );

    SerialUSB.println("FCR");
    SerialUSB.println(  SpiSerial.readRegister(FCR) );

    SerialUSB.println("TXLVL");
    SerialUSB.println( SpiSerial.readRegister(TXLVL) );
    SerialUSB.println("RXLVL");
    SerialUSB.println( SpiSerial.readRegister(RXLVL) );

    SerialUSB.println("try string");


    char bob[]="$$$";
    int i = 0;
    while(bob[i] != '\0'){
        SpiSerial.writeRegister(THR, bob[i]);
        SerialUSB.println(bob[i]);
        i++;
    }
    delay(90);
    SerialUSB.println("TXLVL");
    SerialUSB.println( SpiSerial.readRegister(TXLVL) );
    SerialUSB.println("RXLVL");
    SerialUSB.println( SpiSerial.readRegister(RXLVL) );

    i=0;
    while(bob[i] != '\0'){
        SpiSerial.writeRegister(THR, bob[i]);
        SerialUSB.println(bob[i]);
        i++;
    }
    delay(450);

    char foo[] = "ver";
    i = 0;
    while(foo[i] != '\0'){
        SpiSerial.writeRegister(THR, foo[i]);
        SerialUSB.println(foo[i]);
        i++;
    }

    delay(450);
    SerialUSB.println("TXLVL");
    SerialUSB.println( SpiSerial.readRegister(TXLVL) );
    SerialUSB.println("RXLVL");
    SerialUSB.println( SpiSerial.readRegister(RXLVL) );









//uint32 fifowrite(char * str_out){
}

void loop()
{
    while(SpiSerial.available() > 0) {
        SerialUSB.write(SpiSerial.read());
      }

      if(SerialUSB.available()) { // Outgoing data
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
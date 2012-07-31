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

#include <spi.h>

HardwareSPI spi(1);



uint32_t foobar(const void *buf, uint32 len){
    uint32 txed = 0;
                SerialUSB.print("Showing Buffer of length ");
                SerialUSB.println(len);
    while ( (txed < len)) {

            SerialUSB.println( ((const uint8*)buf)[txed++] , BIN);

    }
    return txed;

}


/**
 * @brief SPI synchronous 8-bit write, blocking.
 * @param spi_num which spi to send on
 * @return data shifted back from the slave
 */
// uint8 spi_tx_byte(uint32 spi_num, uint8 data) {
//     spi_dev *spi;
//
//     spi = (spi_num == 1) ? (spi_dev*)SPI1_BASE : (spi_dev*)SPI2_BASE;
//
//     //returns true wenn  TX register leer ist, daher warten wir bis das TX register leer ist, dann beschreiben wir es wieder
//     while (!(spi->SR & SR_TXE))
//         ;
//
//     spi->DR = data;
//
//     while (!(spi->SR & SR_RXNE))
//         ;
//
//     return spi->DR;
// }

void cnc_info(spi_dev* dev){

    if((dev->regs->CR1 & SPI_CR1_CRCEN) == SPI_CR1_CRCEN_BIT) {
        SerialUSB.println("CNC is on");
    }
    else {
        SerialUSB.println("CNC is off");
    }
}

void setup()
{
  SerialUSB.begin();
  spi.begin(SPI_281_250KHZ, MSBFIRST, 0);
    pinMode(10, OUTPUT);
  SerialUSB.println("type any char to start");



  while (!SerialUSB.available());
  SerialUSB.println("Starting");

    SerialUSB.println("SPI Infos");
    if( spi_dff(spi.c_dev()) == SPI_DFF_8_BIT) {
        SerialUSB.println("8bit frame");
    }
    SerialUSB.println("is cnc on?");
    cnc_info(spi.c_dev());

uint8 foo[2];
SerialUSB.println("Read 3x2 registers");


foo[0] = (0x01<<3) | 0x80;
 digitalWrite(10, LOW);
    SerialUSB.println( spi.transfer( foo[0] ) );
    SerialUSB.println( spi.transfer( 0xff   ) );
 digitalWrite(10, HIGH);

foo[0] = (0x02<<3) | 0x80;
 digitalWrite(10, LOW);
    SerialUSB.println( spi.transfer( foo[0] ) );
    SerialUSB.println( spi.transfer( 0xff   ) );
 digitalWrite(10, HIGH);
SerialUSB.println("0x03 Reg now");
    foo[0] = (0x03<<3) | 0x80;
     digitalWrite(10, LOW);
        SerialUSB.println( spi.transfer( foo[0] ) );
        SerialUSB.println( spi.transfer( 0xff   ) );
     digitalWrite(10, HIGH);


    foo[0] = (0x03 << 3) & 0xff;
    foo[1] = (0x04)  & 0xff;
    digitalWrite(10, LOW);
        SerialUSB.println( spi.write( foo[0] ) );
        SerialUSB.println( spi.write( foo[1] ) );

     digitalWrite(10, HIGH);


     foo[0] = (0x03<<3) | 0x80;
         digitalWrite(10, LOW);
            SerialUSB.println( spi.transfer( foo[0] ) );
            SerialUSB.println( spi.transfer( 0xff   ) );
         digitalWrite(10, HIGH);


    SerialUSB.println("sending 2 byts");


    SerialUSB.println("_______________________");
    SerialUSB.println(foobar(foo, 2));
    SerialUSB.println("_______________________");


    digitalWrite(10, LOW);
    SerialUSB.println( spi.write(foo[0]) );
        SerialUSB.println( spi.write(foo[1]) );
    digitalWrite(10, HIGH);
        delay(10);

    //SerialUSB.println( spi_tx(spi->c_dev(), 0x03, 1) );
    foo[0] = 0x02<<3 | 0x80;

    SerialUSB.println("Read register");



}

void loop()
{
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
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
#include <WiFlyShield/SpiUart.h>
#include <WiFlySerial/WiFlySerial.h>
// #include <WiFlyShield/WiFlyDevice.h>

HardwareSPI spi(1);
SpiUartDevice SpiSerial;
//WiFlyDevice WiFly;
WiFlySerial WiFly;

void setup()
{
	
	spi.begin(SPI_562_500KHZ, MSBFIRST, 0);
    SpiSerial.begin(&spi);
    while (!SerialUSB.available());
    SerialUSB.println("Foo");
    SpiSerial.begin(&spi, 9600);
	delay(100);
	
    WiFly.begin(&SpiSerial);
	
}

void loop()
{
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
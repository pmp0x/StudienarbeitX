#include "WiFlySerial/SpiUart.h"
#include <stdint.h>
#include "libmaple.h"
#include "dma.h"

#include "wirish.h"
#include "HardwareSPI.h"

// See section 8.10 of the datasheet for definitions
// of bits in the Enhanced Features Register (EFR)
#define EFR_ENABLE_CTS 1 << 7
#define EFR_ENABLE_RTS 1 << 6
#define EFR_ENABLE_ENHANCED_FUNCTIONS 1 << 4


// See section 8.4 of the datasheet for definitions
// of bits in the Line Control Register (LCR)
#define LCR_ENABLE_DIVISOR_LATCH 1 << 7


// The original crystal frequency used on the board (~12MHz) didn't
// give a good range of baud rates so around July 2010 the crystal
// was replaced with a better frequency (~14MHz).
#ifndef USE_14_MHZ_CRYSTAL
#define USE_14_MHZ_CRYSTAL true // true (14MHz) , false (12 MHz)
#endif

#if USE_14_MHZ_CRYSTAL
#define XTAL_FREQUENCY 14745600UL // On-board crystal (New mid-2010 Version)
#else
#define XTAL_FREQUENCY 12288000UL // On-board crystal (Original Version)
#endif

// See datasheet section 7.8 for configuring the
// "Programmable baud rate generator"
#define PRESCALER 1 // Default prescaler after reset
#define BAUD_RATE_DIVISOR(baud) ((XTAL_FREQUENCY/PRESCALER)/(baud*16UL))


// TODO: Handle configuration better
// SC16IS750 register values
struct SPI_UART_cfg {
  char DataFormat;
  char Flow;
};

struct SPI_UART_cfg SPI_Uart_config = {
  0x03,
  // We need to enable flow control or we overflow buffers and
  // lose data when used with the WiFly. Note that flow control
  // needs to be enabled on the WiFly for this to work but it's
  // possible to do that with flow control enabled here but not there.
  // TODO: Make this able to be configured externally?
  EFR_ENABLE_CTS | EFR_ENABLE_RTS | EFR_ENABLE_ENHANCED_FUNCTIONS
};

//pointer to spi object
//HardwareSPI *_SPI;
uint8 foo[64];
uint8  bar[64];
SpiUartDevice::SpiUartDevice(){
    
    _iPointer = 0;
    _iData = 0;
    
memset(foo, 0xff, sizeof(_send));
memset(bar, '\0', sizeof(_response));
    
    _response = bar;
    _send = foo;
    
}
/**
	
	@param s The abstract Maple Spi Device
	@param baudrate <#baudrate description#>
 */
void SpiUartDevice::begin(HardwareSPI *s , unsigned long baudrate){
    /*
       * Initialize SPI and UART communications
       *
       * Uses BAUD_RATE_DEFAULT as baudrate if none is given
       */
    _SPI = s;
    pinMode(_SPI->nssPin(),OUTPUT);
    select();
    _SPI->write(0xff);
    deselect();
    delay(20);
    initUart(baudrate);
}


/**

	<#Description#>

 */

void SpiUartDevice::deselect() {
  /*
   * Deslects the SPI device
   */
  digitalWrite(_SPI->nssPin(), HIGH);
}
void SpiUartDevice::select() {
  /*
   * Selects the SPI device
   */
  digitalWrite(_SPI->nssPin(), LOW);
}
	


void SpiUartDevice::initUart(unsigned long baudrate) {
  /*
   * Initialise the UART.
   *
   * If initialisation fails this method does not return.
   */

  // Initialise and test SC16IS750
  configureUart(baudrate);

  if(!uartConnected()){

    // while(1) {
    //             // Lock up if we fail to initialise SPI UART bridge.
    //          };
  }

  // The SPI UART bridge is now successfully initialised.
}


void SpiUartDevice::setBaudRate(unsigned long baudrate) {
  unsigned long divisor = BAUD_RATE_DIVISOR(baudrate);
    // Enable Special Register Set -> Works!
  writeRegister(LCR, LCR_ENABLE_DIVISOR_LATCH); // "Program baudrate"
  //  delay(2);
  writeRegister(DLL, lowByte(divisor));
 //   writeRegister(DLL, 0X41);
    // DLM == DLH
  writeRegister(DLM, highByte(divisor));
//    writeRegister(DLM, 0x0);
}


void SpiUartDevice::configureUart(unsigned long baudrate) {
  /*
   * Configure the settings of the UART.
   */

  // TODO: Improve with use of constants and calculations.
  setBaudRate(baudrate);

  writeRegister(LCR, 0xBF); // access EFR register
//  delay(2);
  writeRegister(EFR, SPI_Uart_config.Flow); // enable enhanced registers

  writeRegister(LCR, SPI_Uart_config.DataFormat); // 8 data bit, 1 stop bit, no parity
//  delay(2);
  writeRegister(FCR, 0x06); // reset TXFIFO, reset RXFIFO, non FIFO mode
  writeRegister(FCR, 0x01); // enable FIFO mode
}


boolean SpiUartDevice::uartConnected() {
  /*
   * Check that UART is connected and operational.
   */

  // Perform read/write test to check if UART is working
  const char TEST_CHARACTER = 'H';
  writeRegister(SPR, TEST_CHARACTER);
  return (readRegister(SPR) == TEST_CHARACTER);
}


void SpiUartDevice::writeRegister(uint8 registerAddress, uint8 data) {
   /*
    * Write <data> byte to the SC16IS750 register <registerAddress>
    */
  select();
  _SPI->transfer(registerAddress);
  _SPI->transfer(data);
  deselect();
}


char SpiUartDevice::readRegister(byte registerAddress) {
  /*
   * Read byte from SC16IS750 register at <registerAddress>.
   */

  // Used in SPI read operations to flush slave's shift register
  	const byte SPI_DUMMY_BYTE = 0xFF;
    char data ;
    select();
//    SerialUSB.println();
//  SerialUSB.print("r");
    _SPI->transfer(SPI_READ_MODE_FLAG | registerAddress);
    //  SerialUSB.print("v");
    data = _SPI->transfer(SPI_DUMMY_BYTE);
    //    SerialUSB.print("x");
    //    SerialUSB.print(registerAddress, HEX);
//    SerialUSB.print(" - ");
//    SerialUSB.print(data2, DEC);
//    SerialUSB.print(" - ");
//   SerialUSB.println(data, DEC);

    deselect();
    return data;
}


uint8 SpiUartDevice::available() {
  /*
   * Get the number of bytes (characters) available for reading.
   *
   * This is data that's already arrived and stored in the receive
   * buffer (which holds 64 bytes).
   */

  // This alternative just checks if there's data but doesn't
  // return how many characters are in the buffer:
  //    readRegister(LSR) & 0x01
    // uint8 size;
    // size = readRegister(RXLVL);
    // if (size > 64) {
    //     flush();
    //     delay(10);
    //
    // }
    // else {
    //     return size;
    // }
    return readRegister(RXLVL);
}


uint8 SpiUartDevice::read(bool dma) {
  /*
   * Read byte from UART.
   *
   * Returns byte read or or -1 if no data available.
   *
   * Acts in the same manner as 'Serial.read()'.
   */
//    if ((_iPointer < _iData + 1) && dma) {
//    
//        return _response[_iPointer++];
//    }
//	uint32 avail = available();
//    if (avail > 6 && dma) {
//        _send[0] = (SPI_READ_MODE_FLAG | RHR);
//		_SPI->RefTransfer(_send, _response);
//    }
//    else if(avail == 0) {
//        return -1;
//    }
	if (!available()) {
        return -1;
    }    
    
    return readRegister(RHR);
}

void SpiUartDevice::bulk_read(uint8 * buf, uint32 size){
    _SPI->transfer(SPI_READ_MODE_FLAG | RHR);
    _SPI->read(buf, size);
}
// char * SpiUartDevice::bulk_read(){
//     uint8 size;
//     size = available();
//     char *buff = malloc(size);
//     if (buff == NULL) return NULL;
//     _SPI->transfer(SPI_READ_MODE_FLAG | RHR);
//     _SPI->read(&buff, (uint32)size);
//     return buff;
// }


void SpiUartDevice::write(byte value) {
  /*
   * Write byte to UART.
   */

  while (readRegister(TXLVL) == 0) {
    // Wait for space in TX buffer
  };
  writeRegister(THR, value);

}


void SpiUartDevice::write(const char *str) {
  /*
   * Write string to UART.
   */
    uint8 size = strlen(str);
  while (size--)
      write(*str++);
      while (readRegister(TXLVL) < 64) {
        // Wait for empty TX buffer (slow)
        // (But apparently still not slow enough to ensure delivery.)
      };

    // if((uint8)readRegister(TXLVL) < size ) {
    //        while(str != '\0'){
    //            write(*str++);
    //            while ((uint8)readRegister(TXLVL) == 64) {
    //              //       // Wait for empty TX buffer (slow)
    //              //       // (But apparently still not slow enough to ensure delivery.)
    //            };
    //        }
    //    }
    //    else {
    //        select();
    //        _SPI->transfer(THR);
    //        _SPI->write(&str, strlen(str));
    //        // while( size > 16 ){
    //        //            _SPI->write(&str, 16);
    //        //            size -=16;
    //        //            str +=16;
    //        //        }
    //        //        _SPI->write(&str, size);
    //        deselect();
    //    }

}


void SpiUartDevice::flush() {
  /*
   * Flush characters from SC16IS750 receive buffer.
   */

  // Note: This may not be the most appropriate flush approach.
  //       It might be better to just flush the UART's buffer
  //       rather than the buffer of the connected device
  //       which is essentially what this does.
  while(available() > 0) {
    read();
  }
}

// void SpiUartDevice::outputRegistrer(){
//
//  for(int i=0; i<16; i++){
//         SerialUSB.print("Reg ");
//        SerialUSB.println(i, HEX);
//        SerialUSB.println(SpiSerial.readRegister(i<<3), BIN);
//      }
//       SerialUSB.println();
//      SerialUSB.println("Special register set");
//      SerialUSB.println();
//
//      SpiSerial.writeRegister(LCR, 1 << 7 );
//      for(int i=0; i<2; i++){
//         SerialUSB.print("Reg ");
//        SerialUSB.println(i, HEX);
//        SerialUSB.println(SpiSerial.readRegister(i<<3), BIN);
//      }
//      SerialUSB.println();
//      SerialUSB.println("Enhanced register Set");
//      SerialUSB.println();
//
//       SpiSerial.writeRegister(LCR, 0xBF);
//      for(int i=0; i<8; i++){
//         SerialUSB.print("Reg ");
//        SerialUSB.println(i, HEX);
//        SerialUSB.println(SpiSerial.readRegister(i<<3), BIN);
//      }
// }

void DMAEvent(){
    
    //we get the DMA event
    dma_irq_cause event = dma_get_irq_cause(DMA1, DMA_CH3);
    
    switch(event) {
            //the event indicates that the transfer was successfully completed
        case DMA_TRANSFER_COMPLETE:
            //11. Disable DMA when we are done
            dma_disable(DMA1,DMA_CH3);
            digitalWrite(10, HIGH);
            SerialUSB.println("Done transfering");
            break;
            //the event indicates that there was an error transmitting
        case DMA_TRANSFER_ERROR:
            //11. Disable DMA when we are done
            dma_disable(DMA1,DMA_CH3);
            digitalWrite(10, HIGH);
            SerialUSB.println("Fail");
            break;
    }
    
}

void SpiUartDevice::init_dma(){
    // 4. Initialize DMA.
    dma_init(DMA1);
    
    //5. Enable DMA to use SPI communication; both TX (output) and RX (input).
    spi_tx_dma_enable(_SPI->c_dev() );
//    //spi_rx_dma_enable(_SPI->c_dev() );
//    for(int i=0; i<512; i++) {
//        bytesToSend[i] = 0xFF;
//    }
    
    // we create a buffer for revceiving the responses
    //byte bytesReceived[512];
    dma_setup_transfer(DMA1, DMA_CH2, &SPI1->regs->DR, DMA_SIZE_8BITS, _send, DMA_SIZE_8BITS, (DMA_MINC_MODE | DMA_TRNS_CMPLT | DMA_TRNS_ERR));
    
    dma_setup_transfer(DMA1, DMA_CH3, &SPI1->regs->DR, DMA_SIZE_8BITS, _response, DMA_SIZE_8BITS,(DMA_MINC_MODE  | DMA_FROM_MEM));
    // 7. Attach an interrupt to the transfer. Note that we need to add
    // the interrupt flag in step 6 (DMA_TRNS_CMPLT and DMA_TRNS_ERR). 
    // Also, we only attach it for one of the transfers since they are 
    // going to finish at the same time because they are in sync.
    
    dma_attach_interrupt(DMA1, DMA_CH3, DMAEvent);
    
    //8. Setup the priority for the DMA transfer.
    dma_set_priority(DMA1, DMA_CH2, DMA_PRIORITY_VERY_HIGH);
    dma_set_priority(DMA1, DMA_CH3, DMA_PRIORITY_VERY_HIGH);

   
    

}

void SpiUartDevice::ioSetDirection(unsigned char bits) {
  writeRegister(IODIR, bits);
}


void SpiUartDevice::ioSetState(unsigned char bits) {
  writeRegister(IOSTATE, bits);
}

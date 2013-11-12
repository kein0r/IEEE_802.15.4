/*******************| Inclusions |*************************************/
#include "IEEE_802.15.4.h"
#include <IEEE_802.15.4_cfg.h>
#include <ioCC2530.h>
#include <board.h>

/**
 * Implementation only covers MAC layer since PHYS layer is completely handled by
 * the MCU in hardware.
*/

/*******************| Macros |*****************************************/

/*******************| Type definitions |*******************************/

/*******************| Global variables |*******************************/

/*******************| Function definition |****************************/

/**
 * ISR for radio Rx interrupt
*/
#pragma vector = RF_VECTOR
__near_func __interrupt void IEE802154_radioRxISR(void)
{
  uint8 overallBufferLength;
  uint8 payloadLength;
  if( RFIRQF0 & IEEE802154_RFIRQF0_RXPKTDONE ) /* A complete frame has been received. */
  {
    /* get overall buffer length since it could be more than one frame in FIFO */
    overallBufferLength = RXFIFOCNT;
    /* handle receive interrupt, first read payload length from rx-buffer. */
    payloadLength = RFD;      
    
    /* ACK frame ? */
    if (payloadLength == RADIO_ACK_PACKET_SIZE)
    {
      
    }
    else /* nope, data frame */
    {
    }
    clearInterruptFlag(RFIRQF0, IEEE802154_RFIRQF0_RXPKTDONE);  // Clear package received interrupt flag
    
    /** TODO: read RSSI or LQI to global variable */
  }
  /* according to (swru191c.pdf) 23.1.2 Interrupt Registers
     To clear an interrupt from the RF Core, one must clear two flags, both the flag 
     set in RF Core and the one set in S1CON or TCON (depending on which interrupt
     is triggered). */
   S1CON = 0;
   IEEE802154_ISFLUSHRX();
}


/**
 * Blocking send of data frame via radio. 
 * @param: payloadLength: length of frame payload excluding header and CRC
 * NOTE: only auto-crc is supported right now
*/
void IEE802154_radioSentDataFrame(IEE802154_DataFrameHeader_t* header, uint8 payloadLength)
{
  uint8 i;
  
  /* todo: check length */

  IEEE802154_ISFLUSHTX();          /* Flush TX FIFO */

  clearInterruptFlag(RFIRQF1, IEEE802154_RFIRQF1_TXDONE);   /* Clear TX interrupt */
  
  /* write length first. Size of header (without pointer to payload) + 
     payloadlength + 2 bytes CRC */
  RFD = (sizeof(IEE802154_DataFrameHeader_t)-1) + payloadLength + IEEE802154_CRCLENGTH;
  
  /* now write IEE 802.15.4 header (without pointer to payload of course) */
  for(i=0;i<sizeof(IEE802154_DataFrameHeader_t)-1 ;i++)
  {
    RFD = ((uint8*)header)[i];
  }
  
  /* finally write paylod to buffer */
  for(i=0;i<payloadLength ;i++)
  {
    RFD = header->payload[i];
  }
  
  // Enable TX after calibration
  IEEE802154_ISTXON();
  
  // wait until transmission is finished
  while ((RFIRQF1 & IEEE802154_RFIRQF1_TXDONE) == 0) ;
  clearInterruptFlag(RFIRQF1, IEEE802154_RFIRQF1_TXDONE);   // Clear TX interrupt
}

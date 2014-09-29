/*******************| Inclusions |*************************************/
#include "IEEE_802.15.4.h"
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
 * Enables needed interrupts for IEEE_802.15.4 radio. Global interrupts are
 * note enabled by this function. Thus, need to call enableAllInterrupt();
 * outside of this function
*/
void IEE802154_radioInit()
{
  
  /* Configure frama handline (FRMCTRL0) use auto ACK and auto CRC for convenience */
  FRMCTRL0 |= (FRMCTRL0_AUTOACK_ENABLED | FRMCTRL0_AUTOCRC_ENABLED);
    
  /* according to (swru191c.pdf) 23.15.1 Register Settings Update
     This section contains a summary of the register settings that must be 
     updated from their default value to have optimal performance.*/
  AGCCTRL1 = 0x15;    /* Adjusts AGC target value */
  TXFILTCFG = 0x09;   /* Sets TX anti-aliasing filter to appropriate bandwidth */
  FSCAL1 = 0x00;      /* Recommended setting for lowest spurious emission */
    
  /* according to (swru191c.pdf) 23.15.3 Register Descriptions
     IEEE 802.15.4-2006 specifies a frequency range from 2405 MHz to 2480 MHz
     with 16 channels 5 MHz apart. The channels are numbered 11 through 26. For an
     IEEE 802.15.4-2006 compliant system, the only valid settings are thus
     FREQ[6:0] = 11 + 5 (channel number – 11).*/   
  FREQCTRL =  FREQCTRL_CHANNEL_OFFSET + FREQCTRL_CHANNEL_FAKTOR * (IEEE802154_Channel - FREQCTRL_CHANNEL_OFFSET);
    
  /* set short address */
  SHORT_ADDR0 = LO_UINT16(IEEE802154_ShortAddress);
  SHORT_ADDR1 = HI_UINT16(IEEE802154_ShortAddress);
    
  /* set PANID */
  PAN_ID0 = LO_UINT16(IEEE802154_PanID);
  PAN_ID1 = HI_UINT16(IEEE802154_PanID);
  
  /* enable general RF interrupt */
  enableInterrupt(IEN2, IEN2_RFIE);
  /* enable rx done interrupt */
  enableInterrupt(RFIRQM0, RFIRQF0_RXPKTDONE);
   
  IEEE802154_ISRFOFF(); /* disables RX/TX and the frequency synthesizer */
  IEEE802154_ISFLUSHRX();
  IEEE802154_ISRXON(); /* enables and calibrates the frequency synthesizer for RX */
}

/**
 * ISR for radio Rx interrupt
*/
#pragma vector = RF_VECTOR
__near_func __interrupt void IEE802154_radioRxISR(void)
{
  uint8_t overallBufferLength;
  uint8_t payloadLength;
  if( RFIRQF0 & RFIRQF0_RXPKTDONE ) /* A complete frame has been received. */
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
    clearInterruptFlag(RFIRQF0, RFIRQF0_RXPKTDONE);  // Clear package received interrupt flag
    
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
 * @param: header: header of frame including pointer to payload
 * @param: payloadLength: length of frame payload excluding header and CRC
 * NOTE: only auto-crc is supported right now
*/
void IEE802154_radioSentDataFrame(IEE802154_DataFrameHeader_t* header, uint8_t payloadLength)
{
  uint8_t i;
  
  /* todo: check length */

  IEEE802154_ISFLUSHTX();          /* Flush TX FIFO */

  clearInterruptFlag(RFIRQF1, RFIRQF1_TXDONE);   /* Clear TX interrupt */
  
  /* write length first. Size of header (without pointer to payload) + 
     payloadlength + 2 bytes CRC */
  RFD = (sizeof(IEE802154_DataFrameHeader_t)-sizeof(uint8_t*)) + payloadLength + IEEE802154_CRCLENGTH;
  
  /* now write IEE 802.15.4 header (without pointer to payload of course) */
  for(i=0;i<sizeof(IEE802154_DataFrameHeader_t)-sizeof(uint8_t*) ;i++)
  {
    RFD = ((uint8_t*)header)[i];
  }
  
  /* finally write paylod to buffer */
  for(i=0;i<payloadLength ;i++)
  {
    RFD = header->payload[i];
  }
  
  // Enable TX after calibration
  IEEE802154_ISTXON();
  
  // wait until transmission is finished
  while ((RFIRQF1 & RFIRQF1_TXDONE) == 0) ;
  clearInterruptFlag(RFIRQF1, RFIRQF1_TXDONE);   // Clear TX interrupt
}
/** @}*/
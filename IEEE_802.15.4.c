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
void IEEE802154_radioInit(IEEE802154_Config_t *config)
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
  FREQCTRL =  FREQCTRL_CHANNEL_OFFSET + FREQCTRL_CHANNEL_FAKTOR * (config->Channel - FREQCTRL_CHANNEL_OFFSET);
    
  /* set short address to configured value and extended address to factory preset. Which value will be used
   * during data reception is defined by frame header */
  SHORT_ADDR0 = LO_UINT16(config->address.shortAddress);
  SHORT_ADDR1 = HI_UINT16(config->address.shortAddress);
  EXT_ADDR0 = IEEE_EXTENDED_ADDRESS0;
  EXT_ADDR1 = IEEE_EXTENDED_ADDRESS1;
  EXT_ADDR2 = IEEE_EXTENDED_ADDRESS2;
  EXT_ADDR3 = IEEE_EXTENDED_ADDRESS3;
  EXT_ADDR4 = IEEE_EXTENDED_ADDRESS4;
  EXT_ADDR5 = IEEE_EXTENDED_ADDRESS5;
  EXT_ADDR6 = IEEE_EXTENDED_ADDRESS6;
  EXT_ADDR7 = IEEE_EXTENDED_ADDRESS7;
    
  /* set PANID */
  PAN_ID0 = LO_UINT16(config->PanID);
  PAN_ID1 = HI_UINT16(config->PanID);
  
  /* enable general RF interrupt */
  enableInterrupt(IEN2, IEN2_RFIE);
  /* enable rx done interrupt */
  enableInterrupt(RFIRQM0, RFIRQF0_RXPKTDONE);
   
  IEEE802154_ISRFOFF(); /* disables RX/TX and the frequency synthesizer */
  IEEE802154_ISFLUSHRX();
  IEEE802154_ISRXON(); /* enables and calibrates the frequency synthesizer for RX */
}

/**
 * ISR for IEEE 802.15.4 radio. ISR must check bits of RFIRQF0 to further check
 * which situation happend. As of now only RXPKTDONE is used for data reception.
 * In case a complete frame hase been received (RXPKTDONE) the ISR will first fill fcf 
 * data, copy RSSI value, copy data to receive buffer and finally call call-back 
 * function depending on which kind of frame was received.
 * Access global variable defined by IEEE 802.15.4 module but definited by application.
*/
#pragma vector = RF_VECTOR
__near_func __interrupt void IEEE802154_radioISR(void)
{
  uint8_t i;
  uint8_t overallBufferLength;
  uint8_t payloadLength;
  uint8_t *rxFramePtr = (uint8_t*)&IEEE802154_RxDataFrame;   /* Pointer used to copy rx data. This is bad, but efficient */
  if( RFIRQF0 & RFIRQF0_RXPKTDONE ) /* A complete frame has been received. */
  {
    /* get overall buffer length since it could be more than one frame in FIFO */
    overallBufferLength = RXFIFOCNT;
    /* handle receive interrupt, first read payload length from rx-buffer. */
    payloadLength = RFD; 
    
    /* Read 2 bytes frame control field, 1 byte sequence number, and 2 bytes destination pan ID */
    for( i=0; i< IEEE802154_HEADERSIZE_STATIC ;i++ )
    {
      *rxFramePtr++ = RFD;
    }
    payloadLength -= IEEE802154_HEADERSIZE_STATIC;
    if (IEEE802154_RxDataFrame.fcf.destinationAddressMode == IEEE802154_FCF_ADDRESS_MODE_16BIT)
    {
      *rxFramePtr++ = RFD;
      *rxFramePtr++ = RFD;
      payloadLength -= sizeof(IEEE802154_ShortAddress_t);
    }
    if (IEEE802154_RxDataFrame.fcf.destinationAddressMode == IEEE802154_FCF_ADDRESS_MODE_64BIT)
    {
      for( i=0; i< sizeof(IEEE802154_ExtendedAddress_t) ;i++ )
      {
        *rxFramePtr++ = RFD;
      }
      payloadLength -= sizeof(IEEE802154_ExtendedAddress_t);
    }
#ifndef IEEE802154_ENABLE_PANID_COMPRESSION
    *rxFramePtr++ = RFD;
    *rxFramePtr++ = RFD;
    payloadLength -= sizeof(IEEE802154_PANIdentifier_t);
#endif
    if (IEEE802154_RxDataFrame.fcf.sourceAddressMode == IEEE802154_FCF_ADDRESS_MODE_16BIT)
    {
      *rxFramePtr++ = RFD;
      *rxFramePtr++ = RFD;
      payloadLength -= sizeof(IEEE802154_ShortAddress_t);
    }
    if (IEEE802154_RxDataFrame.fcf.sourceAddressMode == IEEE802154_FCF_ADDRESS_MODE_64BIT)
    {
      for( i=0; i< sizeof(IEEE802154_ExtendedAddress_t) ;i++ )
      {
        *rxFramePtr++ = RFD;
      }
      payloadLength -= sizeof(IEEE802154_ExtendedAddress_t);
    }
    /* Copy remaining payload, this will add two more bytes with RSSI and Correlation value
     * instead of CRC (see swru191c.pdf Chapter 23.9.7 Frame-Check Sequence) */
    rxFramePtr = (uint8_t*)IEEE802154_RxDataFrame.payload;
    for (i=0; i<payloadLength; i++)
    {
      *rxFramePtr++ = RFD;
    }
    /* Check which frame was received and call appropriate callback */
    if (IEEE802154_RxDataFrame.fcf.frameType == IEEE802154_FCF_FRAME_TYPE_BEACON)
    {
      IEEE802154_UserCbk_BeaconFrameReceived(payloadLength - IEEE802154_CRCLENGTH);
    }
    else if (IEEE802154_RxDataFrame.fcf.frameType == IEEE802154_FCF_FRAME_TYPE_DATA)
    {
      IEEE802154_UserCbk_DataFrameReceived(payloadLength - IEEE802154_CRCLENGTH);
    }
    if (IEEE802154_RxDataFrame.fcf.frameType == IEEE802154_FCF_FRAME_TYPE_ACKNOWLEDGE)
    {
      IEEE802154_UserCbk_AckFrameReceived(payloadLength - IEEE802154_CRCLENGTH);
    }
    else 
    {
      IEEE802154_UserCbk_MACCommandFrameReceived(payloadLength - IEEE802154_CRCLENGTH);
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
void IEEE802154_radioSentDataFrame(IEEE802154_DataFrameHeader_t* header, uint8_t payloadLength)
{
  uint8_t i;
  
  /* todo: check length */

  IEEE802154_ISFLUSHTX();          /* Flush TX FIFO */

  clearInterruptFlag(RFIRQF1, RFIRQF1_TXDONE);   /* Clear TX interrupt */
  
  /* write length first. Size of header (without pointer to payload) + 2 * address 
  (short or extended) payloadlength + 2 bytes CRC */
  uint8_t txLength = IEEE802154_HEADERSIZE_STATIC + IEEE802154_CRCLENGTH;
  if (header->fcf.destinationAddressMode == IEEE802154_FCF_ADDRESS_MODE_64BIT)
  {
    txLength += sizeof(IEEE802154_ExtendedAddress_t);
  }
  if (header->fcf.sourceAddressMode == IEEE802154_FCF_ADDRESS_MODE_64BIT)
  {
    txLength += sizeof(IEEE802154_ExtendedAddress_t);
  }
  if (header->fcf.destinationAddressMode == IEEE802154_FCF_ADDRESS_MODE_16BIT)
  {
    txLength += sizeof(IEEE802154_ShortAddress_t);
  }
  if (header->fcf.sourceAddressMode == IEEE802154_FCF_ADDRESS_MODE_16BIT)
  {
    txLength += sizeof(IEEE802154_ShortAddress_t);
  }
  RFD = txLength;
  
  /* Write 2 bytes frame control field, 1 byte sequence number, and 2 bytes destination pan ID */
  for( i=0; i< IEEE802154_HEADERSIZE_STATIC ;i++ )
  {
    RFD = ((uint8_t*)header)[i];
  }
  if (header->fcf.destinationAddressMode == IEEE802154_FCF_ADDRESS_MODE_64BIT)
  {
    for( i=0; i< sizeof(IEEE802154_ExtendedAddress_t); i++ )
    {
      RFD = header->destinationAddress.extendedAdress[i];
    }
  }
  else if (header->fcf.destinationAddressMode == IEEE802154_FCF_ADDRESS_MODE_16BIT){
    RFD = LO_UINT16(header->destinationAddress.shortAddress);
    RFD = HI_UINT16(header->destinationAddress.shortAddress);
  }
  else {
    /* nothing if IEEE802154_FCF_ADDRESS_MODE_NONE */
  }
#ifndef IEEE802154_ENABLE_PANID_COMPRESSION
  RFD = header->sourcePANID;
#endif
  if (header->fcf.sourceAddressMode == IEEE802154_FCF_ADDRESS_MODE_64BIT)
  {
    for( i=0; i< sizeof(IEEE802154_ExtendedAddress_t); i++ )
    {
      RFD = header->sourceAddress.extendedAdress[i];
    }
  }
  else if (header->fcf.sourceAddressMode == IEEE802154_FCF_ADDRESS_MODE_16BIT) {
    RFD = LO_UINT16(header->sourceAddress.shortAddress);
    RFD = HI_UINT16(header->sourceAddress.shortAddress);
  }
  else {
    /* nothing if IEEE802154_FCF_ADDRESS_MODE_NONE */
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

/** Retransmission of the last frame sent (i.e. in case no ack was received).
 * See Chapter 23.8.4 Retransmission "After a frame has been successfully transmitted, 
 * the FIFO contents are left unchanged. To retransmit the same frame, simply restart 
 * TX by issuing an STXON or STXONCCA command strobe. Note that a retransmission of a 
 * packet is only possible if the packet has been completely transmitted; i.e., a 
 * packet cannot be aborted and then be retransmitted." */
void IEEE802154_retransmit()
{
  // Enable TX after calibration
  IEEE802154_ISTXON();
}
/** @}*/
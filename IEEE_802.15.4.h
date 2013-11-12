/** @ingroup IEEE_802.15.4
 * @{
 */
#ifndef IEEE_802_15_4_H_
#define IEEE_802_15_4_H_

/*******************| Inclusions |*************************************/
#include <PlatformTypes.h>
   
/*******************| Macros |*****************************************/
/**
 * interrupt flag bits 
*/
#define IEEE802154_RFIRQF0_RXPKTDONE            0x40
#define IEEE802154_RFIRQF1_TXDONE               0x02

/**
 * Lenght of CRC in bytes
*/
#define IEEE802154_CRCLENGTH                    (uint8)0x02
   
/**
 * Selected strobes for IEEE 802.15.4. See swru191d.pdf Chapter 23.14 Command 
 * Strobe/CSMA-CA Processor. Quote: "The CSP interfaces with the CPU through the 
 * SFR register RFST and the XREG registers CSPX, CSPY,CSPZ,CSPT,CSPSTAT,CSPCTRL, 
 * and CSPPROG < n > (where n is in the range  0 to 23). The CSP produces 
 * interrupt requests to the CPU. In addition, the CSP interfaces with the MAC 
 * Timer by observing MAC Timer events."
 * For an explanation see swru191d.pdf Chapter 25.3.1.2 Command Register
*/
#define IEEE802154_ISRXON()                     RFST = 0xE3
#define IEEE802154_ISTXON()                     RFST = 0xE9
#define IEEE802154_ISTXONCCA()                  RFST = 0xEA
#define IEEE802154_ISRFOFF()                    RFST = 0xEF
#define IEEE802154_ISFLUSHRX()                  RFST = 0xED  /* was EC?? */
#define IEEE802154_ISFLUSHTX()                  RFST = 0xEE

   
/*******************| Type definitions |*******************************/
/**
  * \brief IEEE 802.15.4 frame control field (fcf)
  * Description of the fields are copied from Contiki project frame802154.h
  */
typedef struct {
  uint16 frameType : 3;               /**< 3 bit. Frame type field, see 802.15.4 */
  uint16 securityEnabled : 1;         /**< 1 bit. True if security is used in this frame */
  uint16 framePending : 1;            /**< 1 bit. True if sender has more data to send */
  uint16 ackRequired : 1;             /**< 1 bit. Is an ack frame required? */
  uint16 panIdCompression : 1;        /**< 1 bit. Is this a compressed header? */
  uint16 unused : 3;                  /**< 3 bit. Unused bits (reserved) */
  uint16 destinationAddressMode : 2;  /**< 2 bit. Destination address mode, see 802.15.4 */
  uint16 frameVersion : 2;            /**< 2 bit. 802.15.4 frame version */
  uint16 SourceAddressMode : 2;       /**< 2 bit. Source address mode, see 802.15.4 */
} IEEE802154_FCF_t;


/**
  * \brief IEEE 802.15.4 frame header according to 802.15.4g-2012 Chapter 5.2.1 General MAC frame format
  * and 5.2.2.2 Data frame format
  */
typedef struct {
  IEEE802154_FCF_t fcf;
  uint8 sequenceNumber;
  uint16 destinationPANID;
  uint16 destinationAddress;
#ifndef IEE802154_USE_PANID_COMPRESSION
  uint16 sourcePANID;
#endif
  uint16 sourceAddress;
  /* todo: add security stuff here */
  uint8 *payload;                     /**< pointer to payload */
} IEE802154_DataFrameHeader_t;

/*******************| Global variables |*******************************/

/*******************| Function prototypes |****************************/

void IEE802154_radioSentDataFrame(IEE802154_DataFrameHeader_t* header, uint8 payloadLength);
#endif

/** @}*/

/** @ingroup IEEE_802.15.4
 * @{
 */
#ifndef IEEE_802_15_4_H_
#define IEEE_802_15_4_H_

/*******************| Inclusions |*************************************/
#include <PlatformTypes.h>
#include <Config.h>
   
/*******************| Macros |*****************************************/
/**
 * Lenght of CRC in bytes
*/
#define IEEE802154_CRCLENGTH                    (uint8_t)0x02

/**
 * According to 802.15.4 "5.2.2.3 Acknowledgment frame format" standard acknowledge frame
 * is 3 bytes plus CRC (2/4 bytes) long
*/
#define IEEE802154_ACK_PACKET_SIZE              (uint8_t)0x03 + IEEE802154_CRCLENGTH

/**
 * Selected strobes for IEEE 802.15.4. See swru191d.pdf Chapter 23.14 Command 
 * Strobe/CSMA-CA Processor. Quote: "The CSP interfaces with the CPU through the 
 * SFR register RFST and the XREG registers CSPX, CSPY,CSPZ,CSPT,CSPSTAT,CSPCTRL, 
 * and CSPPROG < n > (where n is in the range  0 to 23). The CSP produces 
 * interrupt requests to the CPU. In addition, the CSP interfaces with the MAC 
 * Timer by observing MAC Timer events."
 * For an explanation see swru191d.pdf Chapter 23.14.8 Instruction Set Summary and
 * Chapter 23.14.9 23.14.9 Instruction Set Definition
*/
/**
 * The ISRXON instruction immediately enables and calibrates the frequency synthesizer for RX.
*/
#define IEEE802154_ISRXON()                     RFST = 0xE3
/**
 * The ISTXON instruction immediately enables TX after calibration. The instruction waits
 * for the radio to acknowledge the command before executing the next instruction.
*/
#define IEEE802154_ISTXON()                     RFST = 0xE9
/**
 * The ISTXONCCA instruction immediately enables TX after calibration if CCA indicates a clear channel.
*/
#define IEEE802154_ISTXONCCA()                  RFST = 0xEA
/**
 * The ISRFOFF instruction immediately disables RX/TX and the frequency synthesizer.
*/
#define IEEE802154_ISRFOFF()                    RFST = 0xEF
/**
 * The ISFLUSHRX instruction immediately flushes the RXFIFO buffer and resets the demodulator 
*/
#define IEEE802154_ISFLUSHRX()                  RFST = 0xED  /* was EC?? */
/**
* The ISFLUSHTX instruction immediately flushes the TXFIFO buffer.
*/
#define IEEE802154_ISFLUSHTX()                  RFST = 0xEE

/**
 * For detailed explenation see 802.15.4-2006 Chapter "7.2.1.1 Frame Control field"
 * Values for IEEE802154_FCF_t
*/
/* frame type 3bit 0:2 */
#define IEEE802154_FCF_FRAME_TYPE_BEACON            0x00
#define IEEE802154_FCF_FRAME_TYPE_DATA              0x01
#define IEEE802154_FCF_FRAME_TYPE_ACKNOWLEDGE       0x02
#define IEEE802154_FCF_FRAME_TYPE_MAC_COMMAND       0x03
/* security 1 bit 3* */
#define IEEE802154_FCF_SECURITY_DISABLED            0x00
#define IEEE802154_FCF_SECURITY_ENABLED             0x01
/* pending 1 bit 4 */
/* acknowledgement required 1 bit 5 */
#define IEEE802154_FCF_ACKNOWLEDGE_NOT_REQUIRED     0x00
#define IEEE802154_FCF_ACKNOWLEDGE_REQUIRED         0x01
/* pan id compression 1 bit 6*/
#define IEEE802154_FCF_PANIDCOMPRESSION_DISABLED    0x00
#define IEEE802154_FCF_PANIDCOMPRESSION_ENABLED     0x01
/* 3 bit reserved 7-9 */
/* destination address mode 2 bit 10:11 */
/* source address mode 2 bit 14:15 */
#define IEEE802154_FCF_ADDRESS_MODE_NONE            0x00
#define IEEE802154_FCF_ADDRESS_MODE_16BIT           0x02
#define IEEE802154_FCF_ADDRESS_MODE_64BIT           0x03
/* frame version 2 bit 12:13 */

#define IEEE802154_BROADCAST_PAN_ID             (IEEE802154_PANIdentifier_t)0xffff

/**
 * Definition for different register values needed for IEEE 802.15.4 radio
*/
#define FRMCTRL0_SW_CRC_MODE_CORRELATION        0x00
#define FRMCTRL0_SW_CRC_MODE_SRCRESINDEX        0x80
#define FRMCTRL0_AUTOCRC_DISABLED               0x00
#define FRMCTRL0_AUTOCRC_ENABLED                0x40
#define FRMCTRL0_AUTOACK_DISABLED               0x00
#define FRMCTRL0_AUTOACK_ENABLED                0x20
#define FRMCTRL0_ENERGY_SCAN_RECENT             0x00
#define FRMCTRL0_ENERGY_SCAN_PEAK               0x10

#define FREQCTRL_CHANNEL_OFFSET                 (uint8_t)11
#define FREQCTRL_CHANNEL_FAKTOR                 (uint8_t)5

/**
 * interrupt flag bits 
*/
#define RFIRQF0_RXPKTDONE                       0x40
#define RFIRQF1_TXDONE                          0x02
#define IEN2_RFIE                               0x01

/**
 * IEEE 802.15.4 unique IEEE address from the TI range of addresses.
 */
#define IEEE_EXTENDED_ADDRESS0                  XREG( 0x780C )
#define IEEE_EXTENDED_ADDRESS1                  XREG( 0x780D )
#define IEEE_EXTENDED_ADDRESS2                  XREG( 0x780E )
#define IEEE_EXTENDED_ADDRESS3                  XREG( 0x780F )
#define IEEE_EXTENDED_ADDRESS4                  XREG( 0x7810 )
#define IEEE_EXTENDED_ADDRESS5                  XREG( 0x7811 )
#define IEEE_EXTENDED_ADDRESS6                  XREG( 0x7812 )
#define IEEE_EXTENDED_ADDRESS7                  XREG( 0x7813 )

#define IEEE802154_HEADERSIZE_16BITADDRESS      sizeof(IEEE802154_DataFrameHeader_t) - (2 * sizeof(IEEE802154_ExtendedAddress_t)) - sizeof(IEEE802154_PayloadPointer)
#define IEEE802154_HEADERSIZE_64BITADDRESS      sizeof(IEEE802154_DataFrameHeader_t) - (2 * sizeof(IEEE802154_ShortAddress_t)) - sizeof(IEEE802154_PayloadPointer)
#define IEEE802154_HEADERSIZE_STATIC            sizeof(IEEE802154_FCF_t) + sizeof(uint8_t) + sizeof(IEEE802154_PANIdentifier_t)     /**< IEEE 802.15.4 header size without addresses as they may vary but including destination panID */

/*******************| Type definitions |*******************************/

typedef uint16_t IEEE802154_ShortAddress_t;             /**< 16bit short address for IEEE 802.15.4 radio */
typedef uint8_t IEEE802154_ExtendedAddress_t[8];         /**< 64bit short address for IEEE 802.15.4 radio */
typedef uint16_t IEEE802154_PANIdentifier_t;            /**< 16bit PAN Identifier */

/**
  For later use as address type when switching from 16 to 32bit addressing
*/
typedef struct
{
  IEEE802154_ShortAddress_t shortAddress;
  IEEE802154_ExtendedAddress_t extendedAdress;
} IEEE802154_Adress_t;

/**
  * \brief IEEE 802.15.4 config.
  */
typedef struct {
  uint8_t Channel;   /**< Channel to be used for IEEE 802.15.4 radio. The channels are numbered 11 through 26. */
  IEEE802154_Adress_t address;
  IEEE802154_PANIdentifier_t PanID;
} IEEE802154_Config_t;

/*******************| Type definitions |*******************************/
/**
  * \brief IEEE 802.15.4 frame control field (fcf)
  * Description of the fields are copied from Contiki project frame802154.h
  * See also 802.15.4-2006 Chapter "7.2.1.1 Frame Control field"
  */
typedef struct {
  uint16_t frameType : 3;               /**< 3 bit. Frame type field, see 802.15.4 */
  uint16_t securityEnabled : 1;         /**< 1 bit. True if security is used in this frame */
  uint16_t framePending : 1;            /**< 1 bit. True if sender has more data to send */
  uint16_t ackRequired : 1;             /**< 1 bit. Is an ack frame required? */
  uint16_t panIdCompression : 1;        /**< 1 bit. Is this a compressed header? */
  uint16_t unused : 3;                  /**< 3 bit. Unused bits (reserved) */
  uint16_t destinationAddressMode : 2;  /**< 2 bit. Destination address mode, see 802.15.4 */
  uint16_t frameVersion : 2;            /**< 2 bit. 802.15.4 frame version */
  uint16_t sourceAddressMode : 2;       /**< 2 bit. Source address mode, see 802.15.4 */
} IEEE802154_FCF_t;
  
typedef uint8_t IEEE802154_Payload;
typedef IEEE802154_Payload *IEEE802154_PayloadPointer;

/**
  * \brief IEEE 802.15.4 frame header according to 802.15.4g-2012 Chapter 7.2.1 General MAC frame format
  * NOTE: No CRC is added here as hardware will add it automatically
  */
typedef struct {
  IEEE802154_FCF_t fcf;
  uint8_t sequenceNumber;
  IEEE802154_PANIdentifier_t destinationPANID;
  IEEE802154_Adress_t destinationAddress;
#ifndef IEEE802154_ENABLE_PANID_COMPRESSION
  IEEE802154_PANIdentifier_t sourcePANID;
#endif
  IEEE802154_Adress_t sourceAddress;
  /* TODO: add security stuff here */
  IEEE802154_PayloadPointer payload;   /**< pointer to payload */
} IEEE802154_DataFrameHeader_t;

/*******************| Global variables |*******************************/

/*******************| Function prototypes |****************************/
void IEEE802154_radioInit(IEEE802154_Config_t *config);
void IEEE802154_radioSentDataFrame(IEEE802154_DataFrameHeader_t* header, uint8_t payloadLength);
void IEEE802154_retransmit();
#endif

/** @}*/

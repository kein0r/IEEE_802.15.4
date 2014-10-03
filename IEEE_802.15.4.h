/** @ingroup IEEE_802.15.4
 * @{
 */
#ifndef IEEE_802_15_4_H_
#define IEEE_802_15_4_H_

/*******************| Inclusions |*************************************/
#include <PlatformTypes.h>
#include <IEEE_802.15.4_cfg.h>
   
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

#define IEEE802154_USE_64BIT_ADDRESSING         0xfffe

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
 * For an explanation see 802.15.4g-2012.pdf Chapter 5.2.1.1 Frame Control field.
 * Values for IEEE802154_FCF_t
*/
/* frame type 3bit 0:2 */
#define IEEE802154_FRAME_TYPE_BEACON            0x00
#define IEEE802154_FRAME_TYPE_DATA              0x01
#define IEEE802154_FRAME_TYPE_ACKNOWLEDGE       0x02
#define IEEE802154_FRAME_TYPE_MAC_COMMAND       0x03
/* security 1 bit 3* */
#define IEEE802154_SECURITY_DISABLED            0x00
#define IEEE802154_SECURITY_ENABLED             0x01
/* pending 1 bit 4 */
/* acknowledgement required 1 bit 5 */
#define IEEE802154_ACKNOWLEDGE_NOT_REQUIRED     0x00
#define IEEE802154_ACKNOWLEDGE_REQUIRED         0x01
/* pan id compression 1 bit 6*/
#define IEEE802154_PANIDCOMPRESSION_DISABLED    0x01
#define IEEE802154_PANIDCOMPRESSION_ENABLED     0x01
/* 3 bit reserved 7-9 */
/* destination address mode 2 bit 10:11 */
#define IEEE802154_ADDRESS_MODE_NONE            0x00
#define IEEE802154_ADDRESS_MODE_16BIT           0x02
#define IEEE802154_ADDRESS_MODE_64BIT           0x03
/* frame version 2 bit 12:13 */
/* source address mode 2 bit 14:15 */

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


/*******************| Type definitions |*******************************/
/**
  * \brief IEEE 802.15.4 config.
  */
typedef struct {
  uint8_t Channel;   /**< Channel to be used for IEEE 802.15.4 radio. The channels are numbered 11 through 26. */
  uint16_t ShortAddress;
  uint16_t PanID;
} IEEE802154_Config_t;

/*******************| Type definitions |*******************************/
/**
  * \brief IEEE 802.15.4 frame control field (fcf)
  * Description of the fields are copied from Contiki project frame802154.h
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
  uint16_t SourceAddressMode : 2;       /**< 2 bit. Source address mode, see 802.15.4 */
} IEEE802154_FCF_t;

typedef uint8_t IEE802154_Payload;
typedef uint8_t *IEE802154_PayloadPointer;

/**
  * \brief IEEE 802.15.4 frame header according to 802.15.4g-2012 Chapter 5.2.1 General MAC frame format
  * and 5.2.2.2 Data frame format
  * NOTE: No CRC is added here since hardware will add it automatically
  */
typedef struct {
  IEEE802154_FCF_t fcf;
  uint8_t sequenceNumber;
  uint16_t destinationPANID;
  uint16_t destinationAddress;
#ifndef IEEE802154_ENABLE_PANID_COMPRESSION
  uint16_t sourcePANID;
#endif
  uint16_t sourceAddress;
  /* TODO: add security stuff here */
  IEE802154_PayloadPointer payload;   /**< pointer to payload */
} IEE802154_DataFrameHeader_t;

/*******************| Global variables |*******************************/

/*******************| Function prototypes |****************************/
void IEE802154_radioInit(IEEE802154_Config_t *config);
void IEE802154_radioSentDataFrame(IEE802154_DataFrameHeader_t* header, uint8_t payloadLength);
#endif

/** @}*/

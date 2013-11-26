/* mrf89.h*/

/*
 * 26/11/2013 C. E. Prelz <ghub@fluido.as>
 *
 * A Mruby gem for interfacing with an Olimex Olinuxino A13 board
 * (https://www.olimex.com/Products/OLinuXino/A13/A13-OLinuXino)
 *
 * Parameters fot the MRF89XA chip
 */

#ifndef MRF89_H
#define MRF89_H

#include "spi.h"

#define BAND_863
//#define BAND_902
//#define BAND_915

#define DATA_RATE_20
//#define DATA_RATE_200

/*
 * MRF89XA Operating modes
 */

#define RF_SLEEP        0x00
#define RF_STANDBY      0x20
#define RF_SYNTHESIZER  0x40
#define RF_RECEIVER     0x60
#define RF_TRANSMITTER  0x80

#define CHIPMODE_SLEEPMODE 0x00	//000 [7:5]
#define CHIPMODE_STBYMODE  0x20	//001 ;default
#define CHIPMODE_FSMODE    0x40	//010
#define CHIPMODE_RX        0x60	//011
#define CHIPMODE_TX        0x80	//100

#define BITRATE_200     0x00
#define BITRATE_100     0x01
#define BITRATE_66      0x02
#define BITRATE_50      0x03
#define BITRATE_40      0x04
#define BITRATE_25      0x07
#define BITRATE_20      0x09
#define BITRATE_10      0x13
#define BITRATE_5       0x27
#define BITRATE_2       0x63

#define FREQBAND_902    0x00   //902-915 00 [4:3]
#define FREQBAND_915    0x08   //915-928 01 ;default
#define FREQBAND_863    0x10	//950-960 or 863-870 10

#define BW_25  0x00
#define BW_50  0x01
#define BW_75  0x02
#define BW_100 0x03
#define BW_125 0x04
#define BW_150 0x05
#define BW_175 0x06
#define BW_200 0x07
#define BW_225 0x08
#define BW_250 0x09
#define BW_275 0x0A
#define BW_300 0x0B
#define BW_325 0x0C
#define BW_350 0x0D
#define BW_375 0x0E
#define BW_400 0x0F

#define FREQ_DEV_33  0x0B
#define FREQ_DEV_40  0x09
#define FREQ_DEV_50  0x07
#define FREQ_DEV_67  0x05
#define FREQ_DEV_80  0x04
#define FREQ_DEV_100 0x03
#define FREQ_DEV_133 0x02
#define FREQ_DEV_200 0x01

#define VCO_TRIM_00 0x00            // [2:1] Vtune determined by tank inductor values
#define VCO_TRIM_01 0x02
#define VCO_TRIM_10 0x04
#define VCO_TRIM_11 0x06

#define LNA_GAIN_0_DB   0x00        //00 [1:0] 0dB ;default (This is IF Filter gain)
#define LNA_GAIN_45_DB  0x01        //01 -4.5dB
#define LNA_GAIN_9_DB   0x02        //10 -9dB
#define LNA_GAIN_135_DB 0x03        //11 -13.5dB

#define TX_POWER_13_DB      0x00    //[3:1], 13dBm
#define TX_POWER_10_DB      0x01	//10dBm
#define TX_POWER_7_DB       0x02	//7dBm
#define TX_POWER_4_DB       0x03	//4dBm
#define TX_POWER_1_DB       0x04	//1dBm
#define TX_POWER_N_2_DB     0x05	//-2dBm
#define TX_POWER_N_5_DB     0x06	//-5dBm
#define TX_POWER_N_8_DB     0x07	//-8dBm

#define FILCON_SET_65   0x00		//65 KHz
#define FILCON_SET_82   0x10		//82 KHz
#define FILCON_SET_109  0x20
#define FILCON_SET_137  0x30
#define FILCON_SET_157  0x40
#define FILCON_SET_184  0x50
#define FILCON_SET_211  0x60
#define FILCON_SET_234  0x70
#define FILCON_SET_262  0x80
#define FILCON_SET_321  0x90
#define FILCON_SET_378  0xA0
#define FILCON_SET_414  0xB0
#define FILCON_SET_458  0xC0
#define FILCON_SET_514  0xD0
#define FILCON_SET_676  0xE0
#define FILCON_SET_987  0xF0

#if defined(BAND_863)
#  define FREQ_BAND FREQBAND_863        // 863-870 MHz or 950-960 MHz
#  if defined(APPLICATION_CIRCUIT_950)
#    define START_FREQUENCY 950000
#  else
#    define START_FREQUENCY 860000      //in KHz
#  endif
#  define R1CNT 125
#  define P1CNT	100
#  define S1CNT	20                      //center frequency - 868MHz
#elif defined(BAND_902)
#  define FREQ_BAND FREQBAND_902          // 902MHz
#  define START_FREQUENCY 902000
#  define R1CNT 119
#  define P1CNT 99
#  define S1CNT 25                        //center freq - 903MHz
#elif defined(BAND_915)
#  define FREQ_BAND FREQBAND_915        // 915MHz
#  define START_FREQUENCY 915000
#  define R1CNT 119
#  define P1CNT	100
#  define S1CNT	50                      //center frequency - 916MHz
#else
#  error "No valid frequency band defined"
#endif

//Define the R, P, S sets for different channels
#define	RVALUE  100

#if defined(BAND_863)
#  if(!defined(DATA_RATE_100) && !defined(DATA_RATE_200))
#    define CHANNEL_SPACING 300 //Channel spacing 300KHz (except for data rates 100 and 200kbps)
#    define CHANNEL_NUM 32
#    define FULL_CHANNEL_MAP 0xFFFFFFFF
#  else
#    define CHANNEL_SPACING 384 //To support 25 channels instead of 32
#    define CHANNEL_NUM 25
#    define FULL_CHANNEL_MAP 0x01FFFFFF
#  endif
#else
#  define CHANNEL_SPACING 400 //Channel spacing 400KHz
#  define CHANNEL_NUM 32
#  define FULL_CHANNEL_MAP 0xFFFFFFFF
#endif

#define	FXTAL 12.8 //Frequency of the crystal in MHz

#if defined(DATA_RATE_2)
#  define DATARATE BITRATE_2
#  define BANDWIDTH BW_50                
#  define FREQ_DEV FREQ_DEV_33
#  define FILCON_SET FILCON_SET_157	 
#elif defined(DATA_RATE_5)
#  define DATARATE BITRATE_5
#  define BANDWIDTH BW_50                
#  define FREQ_DEV FREQ_DEV_33
#  define FILCON_SET FILCON_SET_157				           
#elif defined(DATA_RATE_10)
#  define DATARATE BITRATE_10
#  define BANDWIDTH BW_50                
#  define FREQ_DEV FREQ_DEV_33
#  define FILCON_SET FILCON_SET_157				                      
#elif defined(DATA_RATE_20)
#  define DATARATE BITRATE_20
#  define BANDWIDTH BW_75                
#  define FREQ_DEV FREQ_DEV_40
#  define FILCON_SET FILCON_SET_234				           
#elif defined(DATA_RATE_40)
#  define DATARATE BITRATE_40
#  define BANDWIDTH BW_150                
#  define FREQ_DEV FREQ_DEV_80
#  define FILCON_SET FILCON_SET_414
#elif defined(DATA_RATE_50)
#  define DATARATE BITRATE_50
#  define BANDWIDTH BW_175                
#  define FREQ_DEV FREQ_DEV_100
#  define FILCON_SET FILCON_SET_514
#elif defined(DATA_RATE_66)
#  define DATARATE BITRATE_66
#  define BANDWIDTH BW_250                
#  define FREQ_DEV FREQ_DEV_133
#  define FILCON_SET FILCON_SET_676
#elif defined(DATA_RATE_100)
#  define DATARATE BITRATE_100
#  define BANDWIDTH BW_400                
#  define FREQ_DEV FREQ_DEV_200
#  define FILCON_SET FILCON_SET_987
#elif defined(DATA_RATE_200)
#  define DATARATE BITRATE_200
#  define BANDWIDTH BW_400
#  define FREQ_DEV FREQ_DEV_200
#  define FILCON_SET FILCON_SET_987
#else
#  error "No valid data rate defined"     
#endif

/*
 * The registers
 */

//General Configuration Registers: Size - 13 Bytes, Start Address - 0x00
#define GCONREG     0x00
#define DMODREG     0x01
#define FDEVREG     0x02
#define BRSREG      0x03
#define FLTHREG     0x04
#define FIFOCREG    0x05
#define R1CREG      0x06
#define P1CREG      0x07
#define S1CREG      0x08
#define R2CREG      0x09
#define P2CREG      0x0A
#define S2CREG      0x0B
#define PACREG      0x0C
//Interrupt Configuration Registers: Size - 3 Bytes, Start Address - 0x0D
#define FTXRXIREG   0x0D
#define FTPRIREG    0x0E
#define RSTHIREG    0x0F
//Receiver Configuration Registers: Size - 6 Bytes, Start Address - 0x10
#define FILCREG     0x10
#define PFCREG      0x11
#define SYNCREG     0x12
#define RESVREG     0x13
#define RSTSREG     0x14
#define OOKCREG     0x15
//Sync Word Configuration Registers: Size - 4 Bytes, Start Address - 0x16
#define SYNCV31REG  0x16
#define SYNCV23REG  0x17
#define SYNCV15REG  0x18
#define SYNCV07REG  0x19
//Transmitter Configuration Registers: Size - 1 Byte, Start Address - 0x1A
#define TXCONREG    0x1A
//Oscillator Configuration Registers: Size - 1 Byte, Start Address - 0x1B
#define CLKOREG     0x1B
//Packet Handling Configuration Registers: Size - 4 Bytes, Start Address - 0x1C
#define PLOADREG    0x1C
#define NADDSREG    0x1D
#define PKTCREG     0x1E
#define FCRCREG     0x1F

#define N_REG (FCRCREG+1)

static const char base_confvals[N_REG]=
{
  CHIPMODE_STBYMODE|FREQ_BAND|VCO_TRIM_11,      //GCONREG
  0x84|LNA_GAIN_0_DB,                     //DMODREG
  FREQ_DEV,                               //FDEVREG
  DATARATE,                               //BRSREG
  0x0C,                                   //FLTHREG
  0xC1,                                   //FIFOCREG
  R1CNT,                                  //R1CREG
  P1CNT,                                  //P1CREG
  S1CNT,                                  //S1CREG
  0,                                      //R2CREG
  0,                                      //P2CREG
  0,                                      //S2CREG
  0x38,                                   //PACREG
  
  0xC8,                                   //FTXRXIREG
  0x0D,                                   //FTPRIREG
  0x00,                                   //RSTHIREG
   
  FILCON_SET|BANDWIDTH,                   //FILCREG   
  0x38,                                   //PFCREG
  0x38,                                   //SYNCREG
  0x07,                                   //RESVREG
  0,                                      //RSTSREG
  0,                                      //OOKCREG
  
  0xef,                                   //SYNCV31REG 
  0xbe,                                   //SYNCV23REG 
  0xad,                                   //SYNCV15REG 
  0xde,                                   //SYNCV7REG
  
  0xF0|(TX_POWER_13_DB<<1),               //TXCONREG
  
  0x88,                                   //CLKOREG
  
  0x40,                                   //PLOADREG
  
  0x00,                                   //NADDSREG
  0xE8,                                   //PKTCREG
  0x00,                                   //FCRCREG
};


#endif

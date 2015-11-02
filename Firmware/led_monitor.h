/* 
 * File:   led_monitor.h
 * Author: celentan
 *
 * Created on October 28, 2013, 1:39 PM
 */

#ifndef LED_MONITOR_H
#define	LED_MONITOR_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <GenericTypeDefs.h>
#ifdef HPS
#define DFLT_NMBR_OF_BOARDS 4
#define DFLT_CH_PER_BOARD 56
#else
#define DFLT_NMBR_OF_BOARDS 6
#define DFLT_CH_PER_BOARD 56
#endif
#define DFLT_NMBR_OF_CH (DFLT_CH_PER_BOARD*DFLT_NMBR_OF_BOARDS)

#define MAX_AMPLITUDE 4095
#define MAX_WIDTH 4095

/*The special ID for ALL channels call*/
#define CH_CALL_ALL -1

/*The id of a null ch in turn_on_off*/
#define BLANK_CH -1


#define DFLT_WIDTH 3000
#define DFLT_AMPL 1000
#define AMPL_WORKAROUND 2000

#define DC_WIDTH 1000




/*in seconds. There are 14 steps in a DC sequence. If we change every 5 minutes, it will take 1h10 minutes for
  a complete iteration. That's ok*/
#define DFLT_TIME_DC_SEQUENCE 300
    
#define SEQ_MAX_STEPS DFLT_NMBR_OF_CH

    typedef enum clk_src{
        INT_CLK=0,
        EXT_CLK
    }clk_src;
   
    typedef enum int_frequency{
        F_8KHz=0,
        F_4KHz,
        F_2KHz,
        F_1KHz,
        F_500Hz,
        F_250Hz,
        F_125Hz,
        F_62Hz
    }int_frequency;

    typedef enum LED_color{
        RED=0,
        BLUE
    }LED_color;



    typedef struct LedSequence{

        int Nsteps; //actual number of programmed steps
        int Nrepetitions; //how many times should we repeat the sequence?

        int curStep;
        int curRepetition;

        char    NledsThisStep[SEQ_MAX_STEPS];
        INT16   IDledsThisStep[SEQ_MAX_STEPS][DFLT_NMBR_OF_BOARDS];
        UINT16  TimeThisStep[SEQ_MAX_STEPS]; //in seconds.

        BOOL isMultiDC; //if false, proceed normally. If true,turn on x4 Led each driver: ID, ID+14, ID+28, ID+42.
        BOOL isConfigured; //has the sequence been configure?
        
        BOOL isOn; //is this sequence currently being processed?


    }LedSequence;


    typedef struct LedScan{
       unsigned short AmplitudeLow[DFLT_NMBR_OF_CH];
       unsigned short AmplitudeDelta[DFLT_NMBR_OF_CH];
       char n_steps[DFLT_NMBR_OF_CH];

       UINT32 LedStatus_low[DFLT_NMBR_OF_BOARDS]; //each of these is a 32 bits variable, corresponding to one driver board. Each bit is a LED. 0:off, 1: on
       UINT32 LedStatus_high[DFLT_NMBR_OF_BOARDS]; //MSB parts of above
     

       int curGroup;
       int curStep;
       int Ton,Toff;     
     
       BOOL isOn; //is this scan currently being processed?
    }LedScan;

    typedef struct LedMonitor{

        BOOL status; //Main System Status. TRUE is on, FALSE is off.

        clk_src FT_clk_src;
        int_frequency FT_int_frequency; 
    
        unsigned short *Amplitude; //each of these is a 16 bit variable, with the Amplitude of a ch
        unsigned short *Width; //as above, for the width
        UINT32 *LedStatus_low; //each of these is a 32 bits variable, corresponding to one driver board. Each bit is a LED. 0:off, 1: on
        UINT32 *LedStatus_high; //MSB parts of above

        LED_color color; //the color to turn ON!
   
        int LedToChange; //an internal variable to control which LED to touch during the concurrent loop
        int LedToChange2; //only for owr==true!
        //this flag controls the behaviour of the system when a LED is turned on,
        //with another one already ON on the same board.
        //owr=FALSE. Ignore command (DEFAULT)
        //owr=TRUE. TURN OFF other LED and TURN on this one.
        BOOL owr;

        //here goes the flags to see if something has been changed, and thus we need to interact with I2C.
        BOOL clkChanged; //the CLK manager has changed (freq OR source)
        BOOL LEDChanged; //has any LED changed its status? 
        BOOL statusChanged; //has the status changed (system ON / OFF)
#ifdef HPS
        BOOL colorChanged; //has the color changed
#endif
        LedSequence MySequence; //The LED sequence
        LedSequence BackupSequence; //It is used during the DC mode to preserve the user loaded one.
        LedScan MyScan;

      }LedMonitor;

    //here goes the functiothat interact with this structure
      LedMonitor* InitLedMonitor();
      int LoadDefaultLedMonitor(LedMonitor *m_monitor);
      int ChangeWidth(int ch,unsigned short newWidth,LedMonitor *m_monitor);
      int ChangeAmplitude(int ch,unsigned short newAmplitude, LedMonitor *m_monitor);

      //some helper function
      int GetBoard(int ch);//given ch, return board ID
      int GetIdInBoard(int ch); //given ch, return ch ID in the BOARD (0..DFLT_CH_PER_BOARD)
      unsigned short GetWidth(int ch,LedMonitor *m_monitor);
      unsigned short GetAmplitude(int ch,LedMonitor *m_monitor);

      BOOL hasBoardOneLEDOn(int board,LedMonitor *m_monitor);
      int  getBoardLEDOn(int board,LedMonitor *MyLedMonitor);
      BOOL isLedOn(int ch,LedMonitor *m_monitor);

     

      //the functions interacting (also) with the I2C system.
      int turn_system_on_off(BOOL turnOn);
      int turn_driver_on_off(int board,BOOL turnOn);
      int turn_on_off(int ch1,int ch2, int ch3, int ch4,BOOL turnOn,LED_color color, unsigned short m_amplitude, unsigned short m_width);
      int sel_clk(clk_src source,int_frequency freq);

      //For LED sequence
      int InitLEDSequence(LedSequence *MySequence,BOOL MultiDC);

      //For LED scan
      BOOL ScanHasOneLEDOn(LedMonitor *m_monitor,int nGroup);
      BOOL ScanHasThisLEDOn(LedMonitor *m_monitor,int nLed);
      short GetAmplScan(LedMonitor *m_monitor,int nLed,int nStep);
      int InitLEDScan(LedScan *MyScan);

      //Decode data from the TFTP server
      int DecodeLedData(LedMonitor *MyLedMonitor,char *Data,int nData,char *str);
      int DecodeLedSequence(LedMonitor *MyLedMonitor,char *Data,int nData,char *str);
#ifdef	__cplusplus
}
#endif

#endif	/* LED_MONITOR_H */


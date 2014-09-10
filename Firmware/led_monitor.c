#include "TCPIPConfig.h"
#include "TCPIP Stack/TCPIP.h"

#include "led_monitor.h"
#include "i2c_support.h"
#include "i2c_defs.h"
#include <stdlib.h>
#include <string.h>
/**
 * Init the LedMonitor structure. Should be used to create the main LedMonitor structure
 * @return A pointer to the newly created LedMonitor structure
 */ 
LedMonitor* InitLedMonitor(){
    LedMonitor* m_monitor;
    m_monitor=malloc(sizeof(LedMonitor));
    //Allocate the arrays with Amplitude and Width
    m_monitor->Amplitude=malloc(sizeof(unsigned short)*DFLT_NMBR_OF_BOARDS*DFLT_CH_PER_BOARD);
    m_monitor->Width=malloc(sizeof(unsigned short)*DFLT_NMBR_OF_BOARDS*DFLT_CH_PER_BOARD);
    m_monitor->LedStatus_low=malloc(sizeof(UINT32)*DFLT_NMBR_OF_BOARDS);
    m_monitor->LedStatus_high=malloc(sizeof(UINT32)*DFLT_NMBR_OF_BOARDS);

    if ((m_monitor->Amplitude==NULL)||(m_monitor->Width==NULL)||(m_monitor->LedStatus_low==NULL)||(m_monitor->LedStatus_high==NULL)){
        m_monitor=NULL;
    }
  
    return m_monitor;
    }
/**
 * Initialize a LedMonitor structure
 * @param m_monitor Pointer to the LedMonitor structure to init
 * @return 1 ok, 0 error
 */
int LoadDefaultLedMonitor(LedMonitor *m_monitor){
    int ret=1;
    int ii=0;


    //put to FALSE all the Changed flags
    m_monitor->clkChanged=FALSE;
    m_monitor->LEDChanged=FALSE;
    m_monitor->statusChanged=FALSE;

    m_monitor->LedToChange=-1;
    
    //system is OFF
    m_monitor->status=FALSE;
    //owr is OFF
    m_monitor->owr=FALSE;
    //freq is 8 KHz, clock is EXT
    m_monitor->FT_clk_src=EXT_CLK;
    m_monitor->FT_int_frequency =F_8KHz;
    //all LEDS are OFF
    for (ii=0;ii<DFLT_NMBR_OF_BOARDS;ii++){
        m_monitor->LedStatus_low[ii]=0x00000000;
        m_monitor->LedStatus_high[ii]=0x00000000;
    }

    //color is RED. Fine also for FT, i.e. enable the proper power (the second being NC)
    m_monitor->color=RED;

    //what about amplitude and width default values?
    for (ii=0;ii<DFLT_NMBR_OF_CH;ii++){
        m_monitor->Amplitude[ii]=DFLT_AMPL;
        m_monitor->Width[ii]=DFLT_WIDTH;
    }

    /*Andrea: don't put this here, since we use TFTP now*/
    // InitLEDSequence(&(m_monitor->MySequence),FALSE);

    InitLEDScan(&(m_monitor->MyScan));
    return ret; //for now.
}
/**
 *
 * @param ch  The channel to change the amplitude of
 * @param newAmplitude The new amplitude, from 0 to MAX_AMPLIDUTE
 * @param m_monitor Pointer to the LedMonitor structure
 * @return 0 error, 1 ok
 */
int ChangeAmplitude(int ch,unsigned short newAmplitude,LedMonitor *m_monitor){
    int ret=1;
    if (ch>DFLT_NMBR_OF_CH) {
        ret=0;
    }
    else if (ch==CH_CALL_ALL){
        int idx=0;
        for (idx=0;idx<DFLT_NMBR_OF_CH;idx++){
            ChangeAmplitude(idx,newAmplitude,m_monitor);
        }
    }
    else if ((newAmplitude<0)||(newAmplitude>MAX_AMPLITUDE)){
        ret=0;
    }
    else{
        m_monitor->Amplitude[ch]=newAmplitude;
        if (isLedOn(ch,m_monitor)) {
            m_monitor->LEDChanged=TRUE;
            m_monitor->LedToChange=ch;            
            }
    }
    return ret;
}



/**
 *
 * @param ch The channel to change width of
 * @param newWidth The new width, from 0 to MAX_WIDTH
 * @param m_monitor Pointer to the LedMonitor structure
 * @return 0 error, 1 ok
 */
int ChangeWidth(int ch,unsigned short newWidth,LedMonitor *m_monitor){
    int ret=1;
    if (ch>=DFLT_NMBR_OF_CH) {
        ret=0;
    }
    else if ((newWidth<0)||(newWidth>MAX_WIDTH)){
        ret=0;
    }
    else if (ch==CH_CALL_ALL){
        int idx=0;
        for (idx=0;idx<DFLT_NMBR_OF_CH;idx++){
            ChangeWidth(idx,newWidth,m_monitor);
        }
    }
    else{
        m_monitor->Width[ch]=newWidth;
        if (isLedOn(ch,m_monitor))  {
            m_monitor->LEDChanged=TRUE;
            m_monitor->LedToChange=ch;
        }
    }
    return ret;
}
/**
 * Return the amplitude of a channel
 * @param ch The channel
 * @param m_monitor Pointer to the LedMonitor structure
 * @return The amplitude
 */
   unsigned short GetAmplitude(int ch,LedMonitor *m_monitor){
       unsigned short ret;
       if ((ch<0)||(ch>=DFLT_NMBR_OF_CH)) {
        ret=-1;
        }
       else ret=m_monitor->Amplitude[ch];
       return ret;
   }

/**
 * Return the width
 * @param ch The channel
 * @param m_monitor Pointer to the LedMonitor structure
 * @return The width
 */
   unsigned short GetWidth(int ch,LedMonitor *m_monitor){
       unsigned short ret;
       if ((ch<0)||(ch>=DFLT_NMBR_OF_CH)) {
        ret=-1;
        }
       else ret=m_monitor->Width[ch];
       return ret;
   }

/**
 *
 * @param ch
 * @return The board hosting this channel, from 0 to 5
 */
 int GetBoard(int ch){
    int bd;
    if ((ch<0)||(ch>=DFLT_NMBR_OF_CH)){
        bd=-1;
    }
    bd=ch/DFLT_CH_PER_BOARD;
    return bd;
}
/**
 *
 * @param ch
 * @return The id of the channel within the board, from 0 to DFLT_CH_PER_BOARD
 */
int GetIdInBoard(int ch){
  int   Inbd=0;
    if ((ch<0)||(ch>=DFLT_NMBR_OF_CH)){
        Inbd=-1;
   }
    Inbd=ch%DFLT_CH_PER_BOARD;
    return Inbd;
}
/**
 * Function to determine if a driver board has already a channel on
 * @param Board the board to check
 * @param m_monitor Pointer to the LedMonitor Structure
 * @return TRUE if there is already a channel on, false if not
 */
BOOL hasBoardOneLEDOn(int board,LedMonitor *m_monitor){
    BOOL flag=FALSE;
    
    UINT32 array_low=m_monitor->LedStatus_low[board]; //this is the array we need to check
    UINT32 array_high=m_monitor->LedStatus_high[board];
    
     if (array_low||array_high) flag=TRUE;

    return flag;
}
/**
 * 
 * @param board The Driver board ID
 * @param MyLedMonitor Pointer to the monitor structure
 * @return The ID of the channel on, from 0 to 55. Return -1 if NO leds on in the board.
 */
 int  getBoardLEDOn(int board,LedMonitor *MyLedMonitor){
     int ret=0;
     if (MyLedMonitor->LedStatus_low[board]){
         while (!((MyLedMonitor->LedStatus_low[board]>>ret)&0x1)) ret++;
         return ret;
     }
     else if (MyLedMonitor->LedStatus_high[board]){
          while (!((MyLedMonitor->LedStatus_high[board]>>ret)&0x1)) ret++;
          ret+=32;
          return ret;
     }
     else return -1;
 }
 /**
  *
  * @param ch The channel to check the status of
  * @param m_monitor Pointer to the monitor structure
  * @return 1 if is on, 0 if is off
  */
BOOL isLedOn(int ch,LedMonitor *m_monitor){
       int board;
       BOOL ret;
       UINT32 bdStatus;

       board=GetBoard(ch);
       ch=GetIdInBoard(ch);
       if (ch<32) {
           bdStatus=(m_monitor->LedStatus_low[board]);
           bdStatus=(bdStatus>>ch)&0x1;
       }
       else{
           bdStatus=(m_monitor->LedStatus_high[board]);
           bdStatus=(bdStatus>>(ch-32))&0x1;
       }
       if (bdStatus) ret=TRUE;
       else ret=FALSE;

       return ret;
}


/*Here go the more evoluted functions to interact with the led_monitor system*/

/**
 *
 * @param board The id of the driver board to turn on (from 0 to 5)
 * @param turnOn Turn it on (1) or off(0)?
 * @param m_monitor The pointer to the LedMonitor structure
 * @return 0 error, 1 ok
 */
int turn_driver_on_off(int board,BOOL turnOn){
  int ret;
  UINT8 hub;
  UINT8 cmd_pca[3]; //outputs OR high Z?
  UINT8 cmd_pca1[3]; //enable LEDs
  UINT8 cmd_dac[2];
  UINT8 jj;


  cmd_pca[0]=PCA_CTRL_REG;
  cmd_pca1[0]=PCA_OUT_REG;
  cmd_pca1[1]=0x00;
  cmd_pca1[2]=0x00;
  cmd_dac[0]=DAC_CMD_MODE_PWR;

  ret=0;
  switch(turnOn){
    case FALSE: //turn this driver board OFF
      cmd_pca[1]=cmd_pca[2]=0xff; //PCA are high Z
      cmd_pca1[2]=0x00; //disable ALL
      cmd_dac[1]=DAC_PWR_DOWN_10k|DAC_PWR_A|DAC_PWR_B; //DAC is OFF
      break;
    case TRUE: //turn this driver board ON
      cmd_pca[1]=cmd_pca[2]=0x00; //PCA are OUTPUTS
      cmd_pca1[2]=0x80; //ENABLE BLUE
      cmd_dac[1]=DAC_PWR_UP|DAC_PWR_A|DAC_PWR_B; //DAC is OFF
      break;
  }
  //1: enable the I2C bus of the board we need to communicate to
  hub=(0x1)<<board;
  ret=I2CTransmitOneByteToAddress(hub,I2C_HUB);
 
  //2: enable / disable the board

  //2a: enable / disable the PCAs
  for (jj=0;jj<4;jj++){
    ret=I2CTransmitMoreBytesToAddress(3,cmd_pca,I2C_PCA|(jj*2)); //PAY ATTENTION, I2C_PCA IT IS CODED IN 8 bits!
    ret=I2CTransmitMoreBytesToAddress(3,cmd_pca1,I2C_PCA|(jj*2));
  }
  //2b: enable-disable the DAC
  ret=I2CTransmitMoreBytesToAddress(2,cmd_dac,I2C_DAC);

 
  //3: disable the I2C bus again, for safety.
  hub=0x00;
  ret=I2CTransmitOneByteToAddress(hub,I2C_HUB);
  return ret;
}


/**
 * Turn on/off a channel (low level function)
 * @param ch1 Which ch to turn on? (from 0 to DFLT_NMBR_OF_CH)
 * @param ch2 As before. It is meaningfull only for DC mode. In pulse mode, put -1
 * @param ch3 As before, same comment.
 * @param ch4 As before, same comment
 * @param turnOn Turn on this channel (TRUE) or turn off (FALSE)?
 * @param color: 0 is red, 1 is blue. (ONLY HPS BOARD!)
 * @param m_amplitude Which amplitude?
 * @param m_width Which width
 * @return 1: ok, 0: error
 */
int turn_on_off(int ch1,int ch2, int ch3,int ch4,BOOL turnOn,LED_color color, unsigned short m_amplitude, unsigned short m_width){
  UINT8 hub,ipca;
  UINT8 cmd_pca[3],cmd_dac[2];
  UINT8 cmd_pca_base[3]; //for the color
  int idBoard1, idInBoard1;
  int idBoard2, idInBoard2;
  int idBoard3, idInBoard3;
  int idBoard4, idInBoard4;
  int tmpCH;
  int ret;
  int ii;
  int tmpCHmulti[4]; //used for the multi-case

  BOOL fMulti=FALSE;
  /*I need to handle the different cases
   -If a ch is BLANK_CH,then put it at the first channel.
   -If it is not BLANK_CH, I can calculate idBoard and idInBoard*/
  if (ch2==BLANK_CH)    {ch2=ch1;} else {fMulti=TRUE;}
  if (ch3==BLANK_CH)    {ch3=ch1;} else {fMulti=TRUE;}
  if (ch4==BLANK_CH)    {ch4=ch1;} else {fMulti=TRUE;}

  
  /*If the first channel is the blank channel, */

  idBoard1=GetBoard(ch1); //this is the id of the board (from 0 to 5)
  idInBoard1=GetIdInBoard(ch1); //this is the id of the channel in the board (from 0 to DFLT_CH_PER_BOARD)
 
  idBoard2=GetBoard(ch2); //this is the id of the board (from 0 to 5)
  idInBoard2=GetIdInBoard(ch2); //this is the id of the channel in the board (from 0 to DFLT_CH_PER_BOARD)

  idBoard3=GetBoard(ch3); //this is the id of the board (from 0 to 5)
  idInBoard3=GetIdInBoard(ch3); //this is the id of the channel in the board (from 0 to DFLT_CH_PER_BOARD)

  idBoard4=GetBoard(ch4); //this is the id of the board (from 0 to 5)
  idInBoard4=GetIdInBoard(ch4); //this is the id of the channel in the board (from 0 to DFLT_CH_PER_BOARD)

  /*Now, if any board is different from the first channel one, return */
  if (fMulti){
    if (idBoard2!=idBoard1) return 0;
    if (idBoard3!=idBoard1) return 0;
    if (idBoard4!=idBoard1) return 0;
  }
  /*Then, if channels are more than 1, and are not in the "proper" configuration, return */
  /*Ch1: 0...13*/
  /*Ch2: 14..27*/
  /*Ch3: 28..41*/
  /*Ch4: 42..56*/
  if (fMulti){
    if ((idInBoard1<0)||(idInBoard1>=14))  return 0;
    if ((idInBoard2<14)||(idInBoard2>=28)) return 0;
    if ((idInBoard3<28)||(idInBoard3>=42)) return 0;
    if ((idInBoard4<42)||(idInBoard4>=56)) return 0;
  }

  cmd_pca[0]=PCA_OUT_REG;
  cmd_pca_base[0]=PCA_OUT_REG;
  /*First, setup the color*/
#ifdef HPS
  if (color==RED){
      cmd_pca_base[1]=0x00;
      cmd_pca_base[2]=0x80;
  }
  else if (color==BLUE){
      cmd_pca_base[1]=0x80;
      cmd_pca_base[2]=0x00;
  }
  else{
      return 0;
  }
#else
    cmd_pca_base[1]=0x00;
    cmd_pca_base[2]=0x80;
#endif
  /*Then the LED to turn on*/
  if (!fMulti){ /* The simplest case: we have only 1 led ON*/
   ipca=(UINT8)((idInBoard1/14)&0x0f); //each PCA controls 14 LEDs on the driver board
   tmpCH=(idInBoard1%14)&0xff; //from 0 to 13
   if (tmpCH<7){ //from 0 to 6
       cmd_pca[1]=cmd_pca_base[1];
       if(turnOn){
          cmd_pca[1]=(cmd_pca[1]|((0x1)<<tmpCH));
      }
      cmd_pca[2]=cmd_pca_base[2];
  }
  else{
    tmpCH-=7; //from 0 to 6
    cmd_pca[1]=cmd_pca_base[1];
    cmd_pca[2]=cmd_pca_base[2];
    if (turnOn){
        cmd_pca[2]=(cmd_pca[2])|((0x1)<<tmpCH);
    }
   }
  }
  else { //we have more than 1 led on!
      tmpCHmulti[0]=(idInBoard1%14)&0xff; //all from 0 to 13
      tmpCHmulti[1]=(idInBoard2%14)&0xff;
      tmpCHmulti[2]=(idInBoard3%14)&0xff;
      tmpCHmulti[3]=(idInBoard4%14)&0xff;
  } //end else if(!fMulti)

   /*0 Workaround. The LED board is designed so that the +12V plane is COMMON. Therefore, each board can power it.
   * This means that switching the color on one driver only is NOT enough!
   */
   /*
   for (ii=0;ii<DFLT_NMBR_OF_BOARDS;ii++){
       hub=(0x1)<<ii;
       ret=I2CTransmitOneByteToAddress(hub,I2C_HUB);
       ret=I2CTransmitMoreBytesToAddress(3,cmd_pca_base,I2C_PCA);
       hub=0x0;
       ret=I2CTransmitOneByteToAddress(hub,I2C_HUB);
    }
    */

  //1: enable the I2C bus of the board we need to communicate to
  hub=(0x1)<<idBoard1;
  ret=I2CTransmitOneByteToAddress(hub,I2C_HUB);

  //2a: Amplitude
  cmd_dac[0]=DAC_LOAD_A|((m_amplitude>>8)&0x0f);
  cmd_dac[1]=m_amplitude&0xff;
  ret=I2CTransmitMoreBytesToAddress(2,cmd_dac,I2C_DAC);
  //2b: Width
  cmd_dac[0]=DAC_LOAD_B|((m_width>>8)&0x0f);
  cmd_dac[1]=m_width&0xff;
  ret=I2CTransmitMoreBytesToAddress(2,cmd_dac,I2C_DAC);
  //2c: Update
  cmd_dac[0]=DAC_UPDATE_ALL;
  ret=I2CTransmitOneByteToAddress(cmd_dac[0],I2C_DAC);


  //3: enable the channel.
  //3a: PCA.
  if (!fMulti)  {
      ret=I2CTransmitMoreBytesToAddress(3,cmd_pca,I2C_PCA|((ipca<<1)&0xe));
#ifdef HPS
      /*We need to make sure that the color is selected. The color is on the first PCA*/
      if (ipca>0) { //previous command did not activate the color
          ipca=0;
          cmd_pca[1]=cmd_pca_base[1]; //only transmit the color
          cmd_pca[2]=cmd_pca_base[2];
          ret=I2CTransmitMoreBytesToAddress(3,cmd_pca,I2C_PCA|((ipca<<1)&0xe));
      }
#endif
  }
  else {
      /*First PCA, first channel*/
      if (tmpCHmulti[0]<7){
          cmd_pca[1]=cmd_pca_base[1];  if (turnOn) cmd_pca[1]=(cmd_pca[1]|((0x1)<<tmpCHmulti[0]));
          cmd_pca[2]=cmd_pca_base[2];
      }
      else{
          cmd_pca[2]=cmd_pca_base[2];  if (turnOn) cmd_pca[2]=(cmd_pca[2]|((0x1)<<(tmpCHmulti[0]-7)));
          cmd_pca[1]=cmd_pca_base[1];
      }
      ret=I2CTransmitMoreBytesToAddress(3,cmd_pca,I2C_PCA|((0x00<<1)&0xe));    //this ensures the color!
      /*Second PCA, second channel*/
      if (tmpCHmulti[1]<7){
          cmd_pca[1]=cmd_pca_base[1];  if (turnOn) cmd_pca[1]=(cmd_pca[1]|((0x1)<<tmpCHmulti[1]));
          cmd_pca[2]=cmd_pca_base[2];
      }
      else{
          cmd_pca[2]=cmd_pca_base[2];  if (turnOn) cmd_pca[2]=(cmd_pca[2]|((0x1)<<(tmpCHmulti[1]-7)));
          cmd_pca[1]=cmd_pca_base[1];
      }
      ret=I2CTransmitMoreBytesToAddress(3,cmd_pca,I2C_PCA|((0x01<<1)&0xe));
      /*Third PCA, third channel*/
      if (tmpCHmulti[2]<7){
          cmd_pca[1]=cmd_pca_base[1];  if (turnOn) cmd_pca[1]=(cmd_pca[1]|((0x1)<<tmpCHmulti[2]));
          cmd_pca[2]=cmd_pca_base[2];
      }
      else{
          cmd_pca[2]=cmd_pca_base[2];  if (turnOn) cmd_pca[2]=(cmd_pca[2]|((0x1)<<(tmpCHmulti[2]-7)));
          cmd_pca[1]=cmd_pca_base[1];
      }
      ret=I2CTransmitMoreBytesToAddress(3,cmd_pca,I2C_PCA|((0x02<<1)&0xe));
       /*Fourth PCA, fourth channel*/
      if (tmpCHmulti[3]<7){
          cmd_pca[1]=cmd_pca_base[1];  if (turnOn) cmd_pca[1]=(cmd_pca[1]|((0x1)<<tmpCHmulti[3]));
          cmd_pca[2]=cmd_pca_base[2];
      }
      else{
          cmd_pca[2]=cmd_pca_base[2];  if (turnOn) cmd_pca[2]=(cmd_pca[2]|((0x1)<<(tmpCHmulti[3]-7)));
          cmd_pca[1]=cmd_pca_base[1];
      }
      ret=I2CTransmitMoreBytesToAddress(3,cmd_pca,I2C_PCA|((0x03<<1)&0xe));
  }
  //4: disable the I2C hub, for safety.
  hub=0x00;
  ret=I2CTransmitOneByteToAddress(hub,I2C_HUB);
  
  return ret;
}



/**
 *
 * @param source The clock source, INT_CLK or EXT_CLK
 * @param freq The (internal) clock source, look at led_monitor.h
 * @return 1 ok, 0 err
 */
int sel_clk(clk_src source,int_frequency freq){
    UINT8 clk_data=0x00;
    UINT8 hub;
    int ret;
	switch (source){
	case INT_CLK:
            clk_data=0x8; //PCF 4th bit (i.e. bit 3 starting from 0) at 1
		break;
	case EXT_CLK:
            clk_data=0x0;
		break;
	default:
                ret=0;
		break;
	}
        if (source==INT_CLK){
        switch (freq){
            case F_62Hz:
                clk_data=clk_data|0x7;
                break;
            case F_125Hz:
                clk_data=clk_data|0x6;
                break;
            case F_250Hz:
                clk_data=clk_data|0x5;
                break;
            case F_500Hz:
                clk_data=clk_data|0x4;
                break;
            case F_1KHz:
                clk_data=clk_data|0x3;
                break;
            case F_2KHz:
                clk_data=clk_data|0x2;
                break;
            case F_4KHz:
                clk_data=clk_data|0x1;
                break;
            case F_8KHz:
                clk_data=clk_data|0x0;
                break;
            default:
                ret=0;
                break;
        }
        }
         hub=0x00;
         ret=I2CTransmitOneByteToAddress(hub,I2C_HUB);/*WORK-AROUND FOR I2C CONFLICT*/
         ret=I2CTransmitOneByteToAddress(clk_data,I2C_CLK_MGR);
	 return ret;
}

/**
 * Turn ON / OFF the whole system
 * @param turnOn TRUE: turn ON, FALSE: turn OFF
 * @return 
 */
int turn_system_on_off(BOOL turnOn){
        int ret=1;
        int ii;

        for (ii=0;ii<DFLT_NMBR_OF_BOARDS;ii++){
            ret=turn_driver_on_off(ii,turnOn);
            if (!ret) break;
        }
 
        return ret;
}

/**
 * Init a LEDSequence structure
 * @param MySequence Pointer to the sequence to init
 * @param MultiDC Is the LED sequence to init a DC sequence? If so, LEDs will stay on continuosly, 4 led x driver.
 * @return
 */
int InitLEDSequence(LedSequence *MySequence,BOOL MultiDC){
    int ii=0;
    int jj=0;
    MySequence->isConfigured=TRUE;
    MySequence->isMultiDC=MultiDC;
    MySequence->Nrepetitions=0; /*By default, only 1 cycle*/
    MySequence->curStep=-1;
    MySequence->curRepetition=0; /*First repetition*/
    MySequence->isOn=FALSE; /*Not on by default*/

    /*Dummy init*/
    if (MultiDC){
        MySequence->Nsteps=DFLT_CH_PER_BOARD/4;
        MySequence->Nrepetitions=-1; /*Infinite loop, turn it off with stop_sequence*/
         for (ii=0;ii<DFLT_CH_PER_BOARD/4;ii++){
             MySequence->TimeThisStep[ii]=1;
             MySequence->NledsThisStep[ii]=DFLT_NMBR_OF_BOARDS;
             for (jj=0;jj<DFLT_NMBR_OF_BOARDS;jj++){
                MySequence->IDledsThisStep[ii][jj]=ii+jj*DFLT_CH_PER_BOARD;
             }
         }
    }
    else{
        MySequence->Nsteps=DFLT_NMBR_OF_CH;
        for (ii=0;ii<DFLT_NMBR_OF_CH;ii++){
            MySequence->TimeThisStep[ii]=1;
            MySequence->NledsThisStep[ii]=1;
            MySequence->IDledsThisStep[ii][0]=ii;
        }
    }
}
/**
 * This is the function that decodes the data from the file obtained via TFTP to
 * LED amplitudes and widths.
 * The data (a stream of char!) is in Data, in the format:
 * nLEDS
 * ID amplitude width
 * ID amplitude width
 * ID amplitude width
 * ...
 *
 * @param MyLedMonitor Pointer to the Monitor Structure
 * @param Data Array of Data
 * @param nData number of chars in Data
 * @param str output string. Already contains "Decode Data"
 * @return 0 if ok, otherwise error
 */
int DecodeLedData(LedMonitor *MyLedMonitor,char *Data,int nData,char *str){
    int nLeds,ii,id;
    unsigned short width,ampl;
 
    char *tmp;
    /*First find leds number*/
    tmp=strtok(Data," \n");
    nLeds=atoi(tmp);

    if (nLeds<=0){
        strcat(str,"BAD number ofLEDS\n");
        return 1;
    }
 

    sprintf(&str[strlen(str)],"nLEDS: %i \n",nLeds);

    for (ii=0;ii<nLeds;ii++){
       tmp=strtok(NULL," \n");
       id=atoi(tmp);
   
       tmp=strtok(NULL," \n");
       ampl=atoi(tmp);

       tmp=strtok(NULL," \n");
       width=atoi(tmp);
       
       if ((id<0)||(id>DFLT_NMBR_OF_CH)) {
           sprintf(&str[strlen(str)],"BAD ID %i\n",id);
           return 1;
       }
       if ((ampl<0)||(ampl>MAX_AMPLITUDE)) {
           strcat(str,"BAD AMPLITUDE\n");
           return 1;
       }
       if ((width<0)||(width>MAX_WIDTH)) {
           strcat(str,"BAD WIDTH\n");
           return 1;
      }
      ChangeWidth(id,width,MyLedMonitor);
      ChangeAmplitude(id,ampl,MyLedMonitor);
    }


    sprintf(&str[strlen(str)],"DONE %i LEDs \n",nLeds);
    return 0;
}
/**
 * This is the function that decodes the data from the file obtained via TFTP to
 * LED amplitudes and widths.
 * The data (a stream of char!) is in Data, in the format:
 * nREPETITIONS
 * nSTEPS
 * nLEDs_step0 time_step0(s) id_led0 id_led1 .. idNLEDs_thisstep0
 * nLEDs_step1 time_step1(s) id_led0 id_led1 .. idNLEDs_thisstep1
 * nLEDs_step2 time_step2(s) id_led0 id_led1 .. idNLEDs_thisstep2
 * ...
 *
 * @param MyLedMonitor Pointer to the Monitor Structure
 * @param Data Array of Data
 * @param nData number of chars in Data
 * @param str output string. Already contains "Decode Data"
 * @return 0 if ok, otherwise error
 */
 int DecodeLedSequence(LedMonitor *MyLedMonitor,char *Data,int nData,char *str){

    int nRep,nSteps,nLeds,ii,jj,kk,id,bd1,bd2;
    int time;
    int pos,length;
    char *tmp;
    
    //use strtok to split.
    tmp=strtok(Data," \n"); // First: nRepetitions
    nRep=atoi(tmp);
    if (nRep<-1){
        strcat(str,"BAD number of repetitions\n");
        return 1;
    }
    sprintf(&str[strlen(str)],"nREPETITIONS: %i \n",nRep);
    MyLedMonitor->MySequence.Nrepetitions=nRep;   

    tmp=strtok(NULL," \n"); //then: nSteps
    nSteps=atoi(tmp);
    if (nSteps<=0){
        strcat(str,"BAD number of steps\n");
        return 1;
    }
    sprintf(&str[strlen(str)],"nSTEPS: %i \n",nSteps);
    MyLedMonitor->MySequence.Nsteps=nSteps;  
    for (ii=0;ii<nSteps;ii++){
       tmp=strtok(NULL," \n"); //then nLeds
       nLeds=atoi(tmp);
       if ((nLeds<1)||(nLeds>DFLT_NMBR_OF_BOARDS)){
           sprintf(&str[strlen(str)],"BAD NUMBER OF LEDS %i %s\n",nLeds,tmp);
           return 1;
       }
       MyLedMonitor->MySequence.NledsThisStep[ii]=nLeds;
       tmp=strtok(NULL," \n"); //then time
       time=atoi(tmp);
       if (time<0){
           sprintf(&str[strlen(str)],"BAD TIME %i\n",time);
           return 1;
       }
       MyLedMonitor->MySequence.TimeThisStep[ii]=time;
       for (jj=0;jj<nLeds;jj++){

        tmp=strtok(NULL," \n"); //then ids
        id=atoi(tmp);
        if ((id<-1)||(id>DFLT_NMBR_OF_CH)) { /*Also -1 is allowed: is the blanck channel, all leds are off*/
           sprintf(&str[strlen(str)],"BAD ID %i\n",id);
           return 1;
        }
        MyLedMonitor->MySequence.IDledsThisStep[ii][jj]=id;
       }
       //Add another check. If there are two LEDs same board, consider only the first and put the second at -1
       for (jj=0;jj<nLeds;jj++){
           if (MyLedMonitor->MySequence.IDledsThisStep[ii][jj]==-1) continue;
           bd1=GetBoard(MyLedMonitor->MySequence.IDledsThisStep[ii][jj]);
           for (kk=jj+1;kk<nLeds;kk++){
               if (MyLedMonitor->MySequence.IDledsThisStep[ii][kk]==-1) continue;
               bd2=GetBoard(MyLedMonitor->MySequence.IDledsThisStep[ii][kk]);
               if (bd1==bd2) MyLedMonitor->MySequence.IDledsThisStep[ii][kk]=-1;
           }
         }
    }
    MyLedMonitor->MySequence.curStep=-1;
    MyLedMonitor->MySequence.isMultiDC=FALSE;
    MyLedMonitor->MySequence.isConfigured=TRUE;
    sprintf(&str[strlen(str)],"DONE SEQUENCE\n");
    return 0;
   }



 /**
  *
  * @param MyScan
  * @param nLed
  * @return
  */
   BOOL ScanHasOneLEDOn(LedMonitor *MyMonitor,int nGroup){
       int ii;
       UINT32 LedStatus=0x00;
       if (nGroup<32){
          for (ii=0;ii<DFLT_NMBR_OF_BOARDS;ii++) LedStatus=LedStatus|MyMonitor->MyScan.LedStatus_low[ii];
          LedStatus=((LedStatus>>nGroup)&0x01);
       }
      else {
           for (ii=0;ii<DFLT_NMBR_OF_BOARDS;ii++) LedStatus=LedStatus|MyMonitor->MyScan.LedStatus_high[ii];
           LedStatus=((LedStatus>>(nGroup-32))&0x01);
      }
      if (LedStatus) return TRUE;
      else return FALSE;
   }

   BOOL ScanHasThisLEDOn(LedMonitor *MyMonitor,int nLed){
       int id=GetBoard(nLed);
       UINT32 LedStatus=0x00;
       if (nLed<32){
          LedStatus=MyMonitor->MyScan.LedStatus_low[id];
          LedStatus=((LedStatus>>nLed)&0x01);
       }
      else {
           LedStatus=MyMonitor->MyScan.LedStatus_high[id];
           LedStatus=((LedStatus>>(nLed-32))&0x01);
      }
      if (LedStatus) return TRUE;
      else return FALSE;
   }


    short GetAmplScan(LedMonitor *m_monitor,int nLed,int nStep){
        short ampl=0;

        char n=m_monitor->MyScan.n_steps[nLed];
        short min=m_monitor->MyScan.AmplitudeLow[nLed];
        short delta=m_monitor->MyScan.AmplitudeDelta[nLed];
  
        short max=delta*n+min;

        if (nStep>=n) ampl=0;
        else ampl=min+delta*nStep;

        if (ampl>4096) {
            ampl=4095;
            m_monitor->MyScan.n_steps[nLed]=nStep+1;
        }
        return ampl;
    
    }



   int InitLEDScan(LedScan *MyScan){

       int ii=0;
       MyScan->isOn=FALSE;
       MyScan->Ton=10;
       MyScan->Toff=5;
       for (ii=0;ii<DFLT_NMBR_OF_BOARDS;ii++){
          MyScan->LedStatus_high[ii]=0x0;
          MyScan->LedStatus_low[ii]=0x0;
       }
   }
       

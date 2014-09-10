#include <peripheral/nvm.h>
#include "TCPIPConfig.h"
#include "TCPIP Stack/TCPIP.h"
#include "GenericTypeDefs.h"
#include "Settings.h"

/**
 * Function used to read the settings from the flash memory.
 * @param m_settings Pointer to a structure holding the settings
 * @return 0 ok, 1 error
 */
int ReadSettingsFromFlash(Settings *m_settings){
    int nRead;
    if (m_settings==NULL){
        return 1;
    }
    nRead=sizeof(Settings);

    UserFlashPageReadBytes(MY_FLASH_PAGE,0x00,(void*)m_settings,nRead);
    return 0;
}

/**
 * Function used to write the settings to the flash memory
 * @param m_settings Pointer to a structure that hold the settings
 * @return 0 ok, 1 error
 */
int WriteSettingsToFlash(Settings *m_settings){
    int nWrite;
    if (m_settings==NULL){
        return 1;
    }
    nWrite=sizeof(Settings);
    UserFlashPageWriteBytes(MY_FLASH_PAGE,0x00,(void*)m_settings,nWrite);
    return 0;
}
     
/*These are the two functions used to read and write flash*/
void UserFlashPageReadBytes(U32 userFlashPage, const U16 addr, void *buff, int numBytes)

{
   // Read numBytes from user page into buff.
   memcpy((void *)buff, (void *)(userFlashPage + addr), numBytes);
}

void UserFlashPageWriteBytes(U32 userFlashPage, const U16 addr, void *data, int numBytes)
{

   U8  pageBuff[BYTE_PAGE_SIZE];
   int numBytesLeft = numBytes;
   int numBytesToWriteToRow;
   int rowIndexInPageBuff;
   if (addr + numBytes > sizeof(pageBuff))
      return;

   // Read entire 4096 byte page (sizeof(pageBuff)) bytes into pageBuff.
   memcpy((void *)pageBuff, (void *)userFlashPage, sizeof(pageBuff));

   // Copy the range of source data bytes into pageBuff
   memcpy((void*)pageBuff,data,numBytes);


 // Erase one page (4096 Bytes) of User Program Flash
 NVMErasePage((void *)userFlashPage);

   // Now program the page with the new data in pageBuff, row by row.
   rowIndexInPageBuff = 0;
   while (numBytesLeft)
   {
      if (numBytesLeft > BYTE_ROW_SIZE)
         numBytesToWriteToRow = BYTE_ROW_SIZE;
      else
         numBytesToWriteToRow = numBytesLeft;
      // Write 512 bytes (one NVM row) from pageBuff. Writes a whole row always.
      NVMWriteRow((void *)(userFlashPage + rowIndexInPageBuff), (void*)pageBuff);
      numBytesLeft       -= numBytesToWriteToRow;
      rowIndexInPageBuff += numBytesToWriteToRow;
   }
}


/**
 * Print settings to str
 * @param str
 */
  void PrintSettings(Settings *m_settings,char *str){
      strcpy(str,"Board settings: \n");
      if (m_settings->useDHCP) strcat(str,"Using DHCP\n");
      else strcat(str,"Using static IP\n");

      sprintf(&str[strlen(str)],"IP: %i.%i.%i.%i \n",m_settings->ip.v[0],m_settings->ip.v[1],m_settings->ip.v[2],m_settings->ip.v[3]);
      sprintf(&str[strlen(str)],"NetMask: %i.%i.%i.%i \n",m_settings->mask.v[0],m_settings->mask.v[1],m_settings->mask.v[2],m_settings->mask.v[3]);
      sprintf(&str[strlen(str)],"Gateway: %i.%i.%i.%i \n",m_settings->gateway.v[0],m_settings->gateway.v[1],m_settings->gateway.v[2],m_settings->gateway.v[3]);
           
      sprintf(&str[strlen(str)],"NetBios name: %s \n",m_settings->NetBIOSName);
      sprintf(&str[strlen(str)],"Data server IP: %i.%i.%i.%i \n",m_settings->data_srv.v[0],m_settings->data_srv.v[1],m_settings->data_srv.v[2],m_settings->data_srv.v[3]);
      sprintf(&str[strlen(str)],"Data file: %s\n",m_settings->DataFileName);
      sprintf(&str[strlen(str)],"Sequence file: %s\n",m_settings->SequenceFileName);
  }




/* 
 * File:   Settings.h
 * Author: celentan
 *
 * Created on December 30, 2013, 10:08 AM
 */

#ifndef SETTINGS_H
#define	SETTINGS_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef U32
#define U32 UINT32
#endif

#ifndef U16
#define U16 UINT16
#endif

#ifndef U8
#define U8 UINT8
#endif

#define MY_FLASH_PAGE 0x9D07F000

    typedef struct Settings{
        IP_ADDR ip; //my ip                                               0x0000
        IP_ADDR mask; //my mask                                           0x0004
        IP_ADDR gateway; //my gateway                                     0x0008
        IP_ADDR data_srv; //the tftp server to load data                  0x000c
        int useDHCP; //use dhcp? Use as int since it is of known size!    0x0010
        char		NetBIOSName[16];          //                      0x0014  - 0x0024
        char            DataFileName[32];     //                          0x0024  - 0x0044
        char            SequenceFileName[32];     //                      0x0044  - 0x0064
        char            InitCheckWord;        //                          0x0064  - 0x0065
        BYTE dummy[BYTE_ROW_SIZE-0x0065]; //                               ends at BYTE_ROW_SIZE
    }Settings;

    void PrintSettings(Settings *m_settings,char *str);
    void UserFlashPageReadBytes(U32 userFlashPage, const U16 addr, void *buff, int numBytes);
    void UserFlashPageWriteBytes(U32 userFlashPage, const U16 addr, void *data, int numBytes);
    int ReadSettingsFromFlash(Settings *m_settings);
    int WriteSettingsToFlash(Settings *m_settings);
#ifdef	__cplusplus
}
#endif

#endif	/* SETTINGS_H */


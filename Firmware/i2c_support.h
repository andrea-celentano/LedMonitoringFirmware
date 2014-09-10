/* 
 * File:   i2c_support.h
 * Author: celentan
 *
 * Created on October 28, 2013, 8:57 AM
 */

#ifndef I2C_SUPPORT_H
#define	I2C_SUPPORT_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <plib.h>


void I2CMainInit();
BOOL I2CStartTransfer(BOOL restart);
void I2CStopTransfer(void);
BOOL I2CTransmitOneByte( UINT8 data );
BOOL I2CTransmitOneByteToAddress(UINT8 data, UINT8 addr);
BOOL I2CTransmitMoreBytes(int DataSz,UINT8* i2cData);
BOOL I2CTransmitMoreBytesToAddress(int DataSz,UINT8 *i2cData, UINT8 addr);

BOOL I2CReceiveBytesFromAddress(UINT8 cmd,int nByte,UINT8 *buffer,UINT8 addr);






#ifdef	__cplusplus
}
#endif

#endif	/* I2C_SUPPORT_H */


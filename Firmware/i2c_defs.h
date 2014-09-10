/* 
 * File:   i2c_defs.h
 * Author: celentan
 *
 * Created on October 31, 2013, 11:58 AM
 */

#ifndef I2C_DEFS_H
#define	I2C_DEFS_H

#ifdef	__cplusplus
extern "C" {
#endif

    //put here all the i2c definitions.

//these are the addresses of the I2C devices
//ALREADY coded on 8 bits. The LSB bit here is 0 (WRITE=1). If you want to READ, OR with 0x01.
#define I2C_HUB 0xE0
#define I2C_CLK_MGR 0x40
#define I2C_DAC 0xB0
#define I2C_PCA 0x40 //these has to be ORd with 0x0,0x1,0x2,0x3 depending on which of the 4 selectors is used


/*Clock source */
#define CLK_BIT 0x8
#define CLK_INT_SRC 0x1
#define CLK_EXT_SRC 0x0

/*PCA COMMANDS*/
#define PCA_OUT_REG 0x02
#define PCA_CTRL_REG 0x06
 /*DAC COMMANDS*/
#define DAC_LOAD_A_UPDATE_ALL 0x00
#define DAC_LOAD_B_UPDATE_ALL 0x10
#define DAC_LOAD_A 0x40
#define DAC_LOAD_B 0x50
#define DAC_UPDATE_ALL_LOAD_A 0x80
#define DAC_UPDATE_ALL_LOAD_B 0x90
#define DAC_UPDATE_ALL_LOAD_ALL 0xC0
#define DAC_LOAD_ALL 0xD0
#define DAC_UPDATE_ALL 0xE0 //don't send second data byte.Don't OR
#define DAC_CMD_MODE_PWR 0xF0 //don't OR
#define DAC_CMD_MODE_READ_A 0xF1 //don't OR
#define DAC_CMD_MODE_READ_B 0xF2 //dont't OR

/*When in CMD_MODE PWR, send second byte xxxx_A_B_P1_P2. Setting A and B to 1 will set corresponding DAC to the power mode corresponding to P1 and P2*/
#define DAC_PWR_UP 0x00
#define DAC_PWR_DOWN 0x01
#define DAC_PWR_DOWN_1k 0x2
#define DAC_PWR_DOWN_10k 0x3
#define DAC_PWR_A 0x8
#define DAC_PWR_B 0x4







#ifdef	__cplusplus
}
#endif

#endif	/* I2C_DEFS_H */


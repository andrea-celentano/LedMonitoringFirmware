#include "i2c_support.h"
#include "HardwareProfile.h"




/*******************************************************************************
  Function:
    BOOL StartTransfer( BOOL restart )

  Summary:
    Starts (or restarts) a transfer to/from the EEPROM.

  Description:
    This routine starts (or restarts) a transfer to/from the EEPROM, waiting (in
    a blocking loop) until the start (or re-start) condition has completed.

  Precondition:
    The I2C module must have been initialized.

  Parameters:
    restart - If FALSE, send a "Start" condition
            - If TRUE, send a "Restart" condition

  Returns:
    TRUE    - If successful
    FALSE   - If a collision occured during Start signaling

  Example:
    <code>
    StartTransfer(FALSE);
    </code>

  Remarks:
    This is a blocking routine that waits for the bus to be idle and the Start
    (or Restart) signal to complete.
  *****************************************************************************/

BOOL I2CStartTransfer( BOOL restart )
{
    I2C_STATUS  status;

    // Send the Start (or Restart) signal
    if(restart)
    {
        I2CRepeatStart(MY_I2C_BUS);
    }
    else
    {
        // Wait for the bus to be idle, then start the transfer
        while( !I2CBusIsIdle(MY_I2C_BUS) );

        if(I2CStart(MY_I2C_BUS) != I2C_SUCCESS)
        {
            DBPRINTF("Error: Bus collision during transfer Start\n");
            return FALSE;
        }
    }

    // Wait for the signal to complete
    do
    {
        status = I2CGetStatus(MY_I2C_BUS);

    } while ( !(status & I2C_START) );

    return TRUE;
}


/*******************************************************************************
  Function:
    void StopTransfer( void )

  Summary:
    Stops a transfer to/from the EEPROM.

  Description:
    This routine Stops a transfer to/from the EEPROM, waiting (in a
    blocking loop) until the Stop condition has completed.

  Precondition:
    The I2C module must have been initialized & a transfer started.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    StopTransfer();
    </code>

  Remarks:
    This is a blocking routine that waits for the Stop signal to complete.
  *****************************************************************************/

void I2CStopTransfer( void )
{
    I2C_STATUS  status;

    // Send the Stop signal
    I2CStop(MY_I2C_BUS);

    // Wait for the signal to complete
    do
    {
        status = I2CGetStatus(MY_I2C_BUS);

    } while ( !(status & I2C_STOP) );
}






/*******************************************************************************
  Function:
    BOOL TransmitOneByte( UINT8 data )

  Summary:
    This transmits one byte.

  Description:
    This transmits one byte via I2C.

  Precondition:
    The transfer must have been previously started.

  Parameters:
    data    - Data byte to transmit

  Returns:
    TRUE    - Data was sent successfully
    FALSE   - A bus collision occured

  Example:
    <code>
    TransmitOneByte(0xAA);
    </code>

  Remarks:
    This is a blocking routine that waits for the transmission to complete.
  *****************************************************************************/

BOOL I2CTransmitOneByte( UINT8 data )
{
    // Wait for the transmitter to be ready
    while(!I2CTransmitterIsReady(MY_I2C_BUS));

    // Transmit the byte
    if(I2CSendByte(MY_I2C_BUS, data) == I2C_MASTER_BUS_COLLISION)
    {
        DBPRINTF("Error: I2C Master Bus Collision\n");
        return FALSE;
    }

    // Wait for the transmission to finish
    while(!I2CTransmissionHasCompleted(MY_I2C_BUS));

    return TRUE;
}
/**
 * This function sends a single byte to an I2C address. 
 * @param data The data to transmit (one byte)
 * @param addr The I2C address, coded in 8 bits, with LSB 0 (i.e. write command)
 * @return TRUE, FALSE depending on the transmition esit
 */

BOOL I2CTransmitOneByteToAddress(UINT8 data, UINT8 addr){
     BOOL Success=TRUE;
     if (!I2CStartTransfer(FALSE)) return FALSE; //wait for the BUS to be ready

     if (!I2CTransmitOneByte(addr)) return FALSE;

     if(!I2CByteWasAcknowledged(MY_I2C_BUS)){
         DBPRINTF("Error: Sent byte was not acknowledged\n");
         Success=FALSE;
         return FALSE;
     }
     if (Success){
        if (!I2CTransmitOneByte(data)) return FALSE;
        if(!I2CByteWasAcknowledged(MY_I2C_BUS)){
            DBPRINTF("Error: Sent byte was not acknowledged\n");
            Success = FALSE;
            }
    }
    I2CStopTransfer();
    return Success;
}

BOOL I2CTransmitMoreBytes(int DataSz,UINT8* i2cData){

    BOOL Success=TRUE;
    int Index=0;

    I2CStartTransfer(FALSE);
   
    while( Success & (Index < DataSz) ){
        // Transmit a byte
        if (I2CTransmitOneByte(i2cData[Index]))
        {
            // Advance to the next byte
            Index++;
        }
        else
        {
            Success = FALSE;
        }

        // Verify that the byte was acknowledged
        if(!I2CByteWasAcknowledged(MY_I2C_BUS))
        {
            DBPRINTF("Error: Sent byte was not acknowledged\n");
            Success = FALSE;
        }
    }
    I2CStopTransfer();
    return Success;
}

/**
 *
 * @param DataSz The number of bytes to send
 * @param i2cData Pointe to the array of data
 * @param addr The I2C address to write to, coded in 8 bits, with LSB set to 0 (write)
 * @return
 */
BOOL I2CTransmitMoreBytesToAddress(int DataSz,UINT8 *i2cData, UINT8 addr){
    BOOL Success=TRUE;
    int Index=0;

    I2CStartTransfer(FALSE);

    I2CTransmitOneByte(addr);
        
    if(!I2CByteWasAcknowledged(MY_I2C_BUS))
        {
            DBPRINTF("Error: Sent byte was not acknowledged\n");
            Success = FALSE;
        }
    while( Success & (Index < DataSz) )
    {
        // Transmit a byte
        if (I2CTransmitOneByte(i2cData[Index]))
        {
            // Advance to the next byte
            Index++;
        }
        else
        {
            Success = FALSE;
        }

        // Verify that the byte was acknowledged
        if(!I2CByteWasAcknowledged(MY_I2C_BUS))
        {
            DBPRINTF("Error: Sent byte was not acknowledged\n");
            Success = FALSE;
        }
    }
    I2CStopTransfer();
    return Success;

}

/**
 *
 * @param cmd  This is the command that is sent to the addr BEFORE the read, to actually initiate the read.
 *             If this is not needed (for example becuase the addr object has only one register to read) put it at 0x00
 * @param nByte The number of bytes to read
 * @param buffer The pointer to the variable where the read data is saved
 * @param addr The address of the object where to read from
 * @return
 */
BOOL I2CReceiveBytesFromAddress(UINT8 cmd,int nByte,UINT8 *buffer,UINT8 addr)
{
    BOOL Success;
    int jj;
    if (cmd!=0x00) {
        I2CTransmitOneByteToAddress(cmd,addr);
    }
    //send again the address, with LSB to 1 (read)
    I2CTransmitOneByte(addr|0x01);

    //start read
    while (I2CReceiverEnable (MY_I2C_BUS, TRUE ) != I2C_SUCCESS ); //enable the receiver

    for (jj=0;jj<nByte;jj++){
        if (I2CReceivedDataIsAvailable(MY_I2C_BUS))
        {
         buffer[jj] = I2CGetByte(MY_I2C_BUS);
         if (jj==(nByte-1)) I2CAcknowledgeByte (MY_I2C_BUS,FALSE);
         else  I2CAcknowledgeByte(MY_I2C_BUS,TRUE);
        }
    }
  

    return TRUE;
}






void I2CMainInit(){
        UINT32 freq;
        I2CConfigure(MY_I2C_BUS, I2C_EN); //void
        freq=I2CSetFrequency(MY_I2C_BUS,GetPeripheralClock(),  I2C_CLOCK_FREQ);
#if defined(PIC32_STARTER_KIT)
        DBPRINTF("I2C actual freq: %i",freq);
#endif
        I2CEnable(MY_I2C_BUS, TRUE); //void
        I2CClearStatus(MY_I2C_BUS, 0x400); //void
}








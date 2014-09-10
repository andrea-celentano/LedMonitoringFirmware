/*********************************************************************
 *
 *	CAN <-> TCP Bridge Example
 *  Module for Microchip TCP/IP Stack
 *	 -Transmits all incoming TCP bytes on a socket out the UART 
 *    module, all incoming UART bytes out of the TCP socket.
 *	 -Reference: None (hopefully AN833 in the future)
 *
 *********************************************************************
 * FileName:        CAN2TCPBridge.c
 * Dependencies:    TCP, Hardware UART module, ARP (optional), 
 *					DNS (optional)
 * Processor:       PIC18, PIC24F, PIC24H, dsPIC30F, dsPIC33F, PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *					Microchip C30 v3.12 or higher
 *					Microchip C18 v3.30 or higher
 *					HI-TECH PICC-18 PRO 9.63PL2 or higher
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2002-2009 Microchip Technology Inc.  All rights
 * reserved.
 *
 * Microchip licenses to you the right to use, modify, copy, and
 * distribute:
 * (i)  the Software when embedded on a Microchip microcontroller or
 *      digital signal controller product ("Device") which is
 *      integrated into Licensee's product; or
 * (ii) ONLY the Software driver source files ENC28J60.c, ENC28J60.h,
 *		ENCX24J600.c and ENCX24J600.h ported to a non-Microchip device
 *		used in conjunction with a Microchip ethernet controller for
 *		the sole purpose of interfacing with the ethernet controller.
 *
 * You should refer to the license agreement accompanying this
 * Software for additional information regarding your rights and
 * obligations.
 *
 * THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 * WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
 * LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * MICROCHIP BE LIABLE FOR ANY INCIDENTAL, SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF
 * PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR SERVICES, ANY CLAIMS
 * BY THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE
 * THEREOF), ANY CLAIMS FOR INDEMNITY OR CONTRIBUTION, OR OTHER
 * SIMILAR COSTS, WHETHER ASSERTED ON THE BASIS OF CONTRACT, TORT
 * (INCLUDING NEGLIGENCE), BREACH OF WARRANTY, OR OTHERWISE.
 *
 *
 * Author               Date		Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Howard Schlunder		06/12/07	Original
 ********************************************************************/
#define __CAN2TCPBRIDGE_C

#include "TCPIPConfig.h"
#include "HardwareProfile.h"

#if defined(STACK_USE_CAN2TCP_BRIDGE)

#define CAN2TCPBRIDGE_PORT	9765

#include "TCPIP Stack/TCPIP.h"

// Comment this define out if we are the server.  
// Insert the appropriate address if we are the client.
//#define USE_REMOTE_TCP_SERVER	"192.168.2.50"

// Ring buffers for transfering data to and from the UART ISR:
//  - (Head pointer == Tail pointer) is defined as an empty FIFO
//  - (Head pointer == Tail pointer - 1), accounting for wraparound,
//    is defined as a completely full FIFO.  As a result, the max data 
//    in a FIFO is the buffer size - 1.
static BYTE vCANRXFIFO[50];
static BYTE vCANTXFIFO[50];
static volatile BYTE *RXHeadPtr = vCANRXFIFO, *RXTailPtr = vCANRXFIFO;
static volatile BYTE *TXHeadPtr = vCANTXFIFO, *TXTailPtr = vCANTXFIFO;


/*********************************************************************
 * Function:        void CAN2TCPBridgeInit(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Sets up the CAN peripheral for this application
 *
 * Note:            Uses interrupts
 ********************************************************************/
void CAN2TCPBridgeInit(void)
{
	CAN_BIT_CONFIG canBitConfig;
    UINT baudPrescalar;

	/* This function will intialize
	 * CAN1 module. */

	/* Step 1: Switch the CAN module
	 * ON and switch it to Configuration
	 * mode. Wait till the switch is 
	 * complete */

	CANEnableModule(CAN1,TRUE);

	CANSetOperatingMode(CAN1, CAN_CONFIGURATION);
	while(CANGetOperatingMode(CAN1) != CAN_CONFIGURATION);			

	/* Step 2: Configure the CAN Module Clock. The
	 * CAN_BIT_CONFIG data structure is used
	 * for this purpose. The propagation segment, 
	 * phase segment 1 and phase segment 2
	 * are configured to have 3TQ. The
     * CANSetSpeed function sets the baud.*/
	
	canBitConfig.phaseSeg2Tq            = CAN_BIT_3TQ;
	canBitConfig.phaseSeg1Tq            = CAN_BIT_3TQ;
	canBitConfig.propagationSegTq       = CAN_BIT_3TQ;
	canBitConfig.phaseSeg2TimeSelect    = TRUE;
	canBitConfig.sample3Time            = TRUE;
    canBitConfig.syncJumpWidth          = CAN_BIT_2TQ;

    CANSetSpeed(CAN1,&canBitConfig,SYSTEM_FREQ,CAN_BUS_SPEED);
  
	/* Step 3: Assign the buffer area to the
     * CAN module.
     */ 

	CANAssignMemoryBuffer(CAN1,CAN1MessageFifoArea,(2 * 8 * 16));	

	/* Step 4: Configure channel 0 for TX and size of
     * 8 message buffers with RTR disabled and low medium
     * priority. Configure channel 1 for RX and size
     * of 8 message buffers and receive the full message.
     */

    CANConfigureChannelForTx(CAN1, CAN_CHANNEL0, 8, CAN_TX_RTR_DISABLED, CAN_LOW_MEDIUM_PRIORITY);
    CANConfigureChannelForRx(CAN1, CAN_CHANNEL1, 8, CAN_RX_FULL_RECEIVE);
	
	/* Step 5: Configure filters and mask. Configure
     * filter 0 to accept SID messages with ID 0x201.
     * Configure filter mask 0 to compare all the ID
     * bits and to filter by the ID type specified in
     * the filter configuration. Messages accepted by
     * filter 0 should be stored in channel 1. */

    CANConfigureFilter      (CAN1, CAN_FILTER0, 0x201, CAN_SID);    
    CANConfigureFilterMask  (CAN1, CAN_FILTER_MASK0, 0x00, CAN_SID, CAN_FILTER_MASK_IDE_TYPE);
    CANLinkFilterToChannel  (CAN1, CAN_FILTER0, CAN_FILTER_MASK0, CAN_CHANNEL1); 
    CANEnableFilter         (CAN1, CAN_FILTER0, TRUE);
	
	/* Step 6: Enable interrupt and events. Enable the receive
     * channel not empty event (channel event) and the receive
     * channel event (module event).
     * The interrrupt peripheral library is used to enable
     * the CAN interrupt to the CPU. */

    CANEnableChannelEvent(CAN1, CAN_CHANNEL1, CAN_RX_CHANNEL_NOT_EMPTY, TRUE);
	CANEnableChannelEvent(CAN1, CAN_CHANNEL0, CAN_TX_CHANNEL_NOT_EMPTY, TRUE);
    CANEnableModuleEvent (CAN1, (CAN_TX_EVENT | CAN_RX_EVENT), TRUE);

    /* These functions are from interrupt peripheral
     * library. */
    
    INTSetVectorPriority(INT_CAN_1_VECTOR, INT_PRIORITY_LEVEL_4);
    INTSetVectorSubPriority(INT_CAN_1_VECTOR, INT_SUB_PRIORITY_LEVEL_0);
//    INTEnable(INT_CAN1, INT_ENABLED);

	/* Step 7: Switch the CAN mode
	 * to normal mode. */

	CANSetOperatingMode(CAN1, CAN_NORMAL_OPERATION);
	while(CANGetOperatingMode(CAN1) != CAN_NORMAL_OPERATION);

}


/*********************************************************************
 * Function:        void CAN2TCPBridgeTask(void)
 *
 * PreCondition:    Stack is initialized()
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
void CAN2TCPBridgeTask(void)
{
	static enum _BridgeState
	{
		SM_HOME = 0,
		SM_SOCKET_OBTAINED
	} BridgeState = SM_HOME;
	static TCP_SOCKET MySocket = INVALID_SOCKET;
	WORD wMaxPut, wMaxGet, w;
	BYTE *RXHeadPtrShadow, *RXTailPtrShadow;
	BYTE *TXHeadPtrShadow, *TXTailPtrShadow;


	switch(BridgeState)
	{
		case SM_HOME:
			#if defined(USE_REMOTE_TCP_SERVER)
				// Connect a socket to the remote TCP server
				MySocket = TCPOpen((DWORD)USE_REMOTE_TCP_SERVER, TCP_OPEN_ROM_HOST, CAN2TCPBRIDGE_PORT, TCP_PURPOSE_CAN_2_TCP_BRIDGE);
			#else
			MySocket = TCPOpen(0, TCP_OPEN_SERVER, UART2TCPBRIDGE_PORT, TCP_PURPOSE_CAN_2_TCP_BRIDGE);
			#endif

			
			// Abort operation if no TCP socket of type TCP_PURPOSE_CAN_2_TCP_BRIDGE is available
			// If this ever happens, you need to go add one to TCPIPConfig.h
			if(MySocket == INVALID_SOCKET)
				break;

			// Eat the first TCPWasReset() response so we don't 
			// infinitely create and reset/destroy client mode sockets
			TCPWasReset(MySocket);
			
			// We have a socket now, advance to the next state
			BridgeState = SM_SOCKET_OBTAINED;
			break;

		case SM_SOCKET_OBTAINED:
			// Reset all buffers if the connection was lost
			if(TCPWasReset(MySocket))
			{
				// Optionally discard anything in the CAN FIFOs
				//RXHeadPtr = vUARTRXFIFO;
				//RXTailPtr = vUARTRXFIFO;
				//TXHeadPtr = vUARTTXFIFO;
				//TXTailPtr = vUARTTXFIFO;
				
				// If we were a client socket, close the socket and attempt to reconnect
				#if defined(USE_REMOTE_TCP_SERVER)
					TCPDisconnect(MySocket);
					MySocket = INVALID_SOCKET;
					BridgeState = SM_HOME;
					break;
				#endif
			}
		
			// Don't do anything if nobody is connected to us
			if(!TCPIsConnected(MySocket))
				break;
			

			// Read FIFO pointers into a local shadow copy.  Some pointers are volatile 
			// (modified in the ISR), so we must do this safely by disabling interrupts
			RXTailPtrShadow = (BYTE*)RXTailPtr;
			TXHeadPtrShadow = (BYTE*)TXHeadPtr;
				
//			IEC1bits.U2RXIE = 0;
//			IEC1bits.U2TXIE = 0;

			INTEnable(INT_CAN1, INT_DISABLED);

			RXHeadPtrShadow = (BYTE*)RXHeadPtr;
			TXTailPtrShadow = (BYTE*)TXTailPtr;
			
//			IEC1bits.U2RXIE = 1;
//			if(TXHeadPtrShadow != TXTailPtrShadow)
//				IEC1bits.U2TXIE = 1;
			INTEnable(INT_CAN1, INT_ENABLED); 

			//
			// Transmit pending data that has been placed into the UART RX FIFO (in the ISR)
			//
			wMaxPut = TCPIsPutReady(MySocket);	// Get TCP TX FIFO space
			wMaxGet = RXHeadPtrShadow - RXTailPtrShadow;	// Get UART RX FIFO byte count
			if(RXHeadPtrShadow < RXTailPtrShadow)
				wMaxGet += sizeof(vCANRXFIFO);
			if(wMaxPut > wMaxGet)				// Calculate the lesser of the two
				wMaxPut = wMaxGet;
			if(wMaxPut)							// See if we can transfer anything
			{
				// Transfer the data over.  Note that a two part put 
				// may be needed if the data spans the vUARTRXFIFO 
				// end to start address.
				w = vCANRXFIFO + sizeof(vCANRXFIFO) - RXTailPtrShadow;
				if(wMaxPut >= w)
				{
					TCPPutArray(MySocket, RXTailPtrShadow, w);
					RXTailPtrShadow = vCANRXFIFO;
					wMaxPut -= w;
				}
				TCPPutArray(MySocket, RXTailPtrShadow, wMaxPut);
				RXTailPtrShadow += wMaxPut;

				// No flush.  The stack will automatically flush and do 
				// transmit coallescing to minimize the number of TCP 
				// packets that get sent.  If you explicitly call TCPFlush()
				// here, latency will go down, but so will max throughput 
				// and bandwidth efficiency.
			}

			//
			// Transfer received TCP data into the CAN TX FIFO for future transmission (in the ISR)
			//
			wMaxGet = TCPIsGetReady(MySocket);	// Get TCP RX FIFO byte count
			wMaxPut = TXTailPtrShadow - TXHeadPtrShadow - 1;// Get CAN TX FIFO free space
			if(TXHeadPtrShadow >= TXTailPtrShadow)
				wMaxPut += sizeof(vCANTXFIFO);
			if(wMaxPut > wMaxGet)				// Calculate the lesser of the two
				wMaxPut = wMaxGet;
			if(wMaxPut)							// See if we can transfer anything
			{
				// Transfer the data over.  Note that a two part put 
				// may be needed if the data spans the vUARTTXFIFO 
				// end to start address.
				w = vCANTXFIFO + sizeof(vCANTXFIFO) - TXHeadPtrShadow;
				if(wMaxPut >= w)
				{
					TCPGetArray(MySocket, TXHeadPtrShadow, w);
					TXHeadPtrShadow = vCANTXFIFO;
					wMaxPut -= w;
				}
				TCPGetArray(MySocket, TXHeadPtrShadow, wMaxPut);
				TXHeadPtrShadow += wMaxPut;
			}
			
			// Write local shadowed FIFO pointers into the volatile FIFO pointers.
			
//			IEC1bits.U2RXIE = 0;
//			IEC1bits.U2TXIE = 0;
			INTEnable(INT_CAN1, INT_DISABLED);
			
			RXTailPtr = (volatile BYTE*)RXTailPtrShadow;
			TXHeadPtr = (volatile BYTE*)TXHeadPtrShadow;
			
//			IEC1bits.U2RXIE = 1;
//			if(TXHeadPtrShadow != TXTailPtrShadow)
//				IEC1bits.U2TXIE = 1;
			INTEnable(INT_CAN1, INT_ENABLED);

			break;
	}
}

/*********************************************************************
 * Function:        void _ISR _CAN1Interrupt(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Copies bytes to and from the local CAN TX and 
 *					RX FIFOs
 *
 * Note:            None
 ********************************************************************/
 
void __attribute__((vector(46), interrupt(ipl4), nomips16)) CAN1InterruptHandler(void)
{
	/* This is the CAN1 Interrupt Handler.
	 * Note that there are many source events in the
	 * CAN1 module for this interrupt. These
	 * events are enabled by the  CANEnableModuleEvent()
     * function. In this example, only the RX_EVENT
	 * is enabled. */


	/* Check if the source of the interrupt is
	 * RX_EVENT. This is redundant since only this
     * event is enabled in this example but
     * this shows one scheme for handling events. */

	if((CANGetModuleEvent(CAN1) & CAN_RX_EVENT) != 0)
	{
		
		/* Within this, you can check which channel caused the 
		 * event by using the CANGetModuleEvent() function
         * which returns a code representing the highest priority
         * pending event. */ 
		
		if(CANGetPendingEventCode(CAN1) == CAN_CHANNEL1_EVENT)
		{
			/* This means that channel 1 caused the event.
			 * The CAN_RX_CHANNEL_NOT_EMPTY event is persistent. You
			 * could either read the channel in the ISR
			 * to clear the event condition or as done 
			 * here, disable the event source, and set
			 * an application flag to indicate that a message
			 * has been received. The event can be
			 * enabled by the application when it has processed
			 * one message.
			 *
			 * Note that leaving the event enabled would
			 * cause the CPU to keep executing the ISR since
			 * the CAN_RX_CHANNEL_NOT_EMPTY event is persistent (unless
			 * the not empty condition is cleared.) 
			 * */

			CANRxMessageBuffer * message;
			message = CANGetRxMessage(CAN1,CAN_CHANNEL1);

			// Copy the byte into the local FIFO, if it won't cause an overflow
			if(RXHeadPtr != RXTailPtr - 1)
			{
				if((RXHeadPtr != vCANRXFIFO + sizeof(vCANRXFIFO)) || (RXTailPtr != vCANRXFIFO))
				{
					*RXHeadPtr++ = message->Data[0];
					if(RXHeadPtr >= vCANRXFIFO + sizeof(vCANRXFIFO))
						RXHeadPtr = vCANRXFIFO;
				}
			}
			CANUpdateChannel(CAN1, CAN_CHANNEL1);	
		}
		INTClearFlag(INT_CAN1);
	}
	else if((CANGetModuleEvent(CAN1) & CAN_TX_EVENT) != 0)
	{
		if(CANGetPendingEventCode(CAN1) == CAN_CHANNEL0_EVENT)
		{
			CANTxMessageBuffer * message;
			/* Get a pointer to the next buffer in the channel
     		 * check if the returned value is null. */

			message = CANGetTxMessageBuffer(CAN1,CAN_CHANNEL0);
			
			// Transmit a byte, if pending, if possible
			if(TXHeadPtr != TXTailPtr)
			{
				if(message != NULL)
			    {

			        /* Form a Standard ID CAN message.
			         * Start by clearing the buffer.  
			         * Send message to CAN2.				
			         * IDE = 0 means Standard ID message.	
			         * Send one byte of data.				
			         * This is the payload.					*/


			        message->messageWord[0] = 0;
			        message->messageWord[1] = 0;
			        message->messageWord[2] = 0;
			        message->messageWord[3] = 0;

			        message->msgSID.SID 	= 0x202;		
			        message->msgEID.IDE 	= 0;			
			        message->msgEID.DLC 	= 1;			
			        message->data[0]        = *TXTailPtr++;	

			        /* This function lets the CAN module
			         * know that the message processing is done
			         * and message is ready to be processed. */

			        CANUpdateChannel(CAN1,CAN_CHANNEL0);

			        /* Direct the CAN module to flush the
			         * TX channel. This will send any pending
			         * message in the TX channel. */

			        CANFlushTxChannel(CAN1,CAN_CHANNEL0);
					
					if(TXTailPtr >= vCANTXFIFO + sizeof(vCANTXFIFO))
					TXTailPtr = vCANTXFIFO;
			    }
				INTClearFlag(INT_CAN1);
			}
			else	// Disable the TX interrupt if we are done so that we don't keep entering this ISR
			{
				INTEnable(INT_CAN1, INT_DISABLED);
			}
		}
	}
	INTClearFlag(INT_CAN1);
}

#endif	//#if defined(STACK_USE_CAN2TCP_BRIDGE)


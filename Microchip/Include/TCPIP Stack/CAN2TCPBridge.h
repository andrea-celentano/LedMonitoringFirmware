/*********************************************************************
 *
 *	CAN <-> TCP Bridge Example
 *  Module for Microchip TCP/IP Stack
 *	 -Transmits all incoming TCP bytes on a socket out the CAN 
 *    module, all incoming CAN bytes out of the TCP socket.
 *	 -Reference: None (hopefully AN833 in the future)
 *
 *********************************************************************
 * FileName:        CAN2TCPBridge.c
 * Dependencies:    TCP.h
 * Processor:       PIC32
 * Compiler:        Microchip C32 v1.05 or higher
 *
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * Copyright (C) 2010 Microchip Technology Inc.  All rights
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
 * Author               Date    	Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Aseem Swalah		   12/20/10	    Original
 ********************************************************************/
#ifndef __CAN2TCPBRIDGE_H
#define __CAN2TCPBRIDGE_H

#include "plib.h"

#define SYSTEM_FREQ 80000000
#define CAN_BUS_SPEED 250000

#define CAN2TCPBRIDGE_PORT	9765

BYTE CAN1MessageFifoArea[2 * 8 * 16];
BYTE CAN2MessageFifoArea[2 * 8 * 16];
								
void CAN2TCPBridgeInit(void);
void CAN2TCPBridgeTask(void);
void CAN2TCPBridgeISR(void);

#endif


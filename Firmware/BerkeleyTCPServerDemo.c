/*********************************************************************
 *
 *  Berekely TCP server demo application.
 *  This application uses the BSD socket APIs and starts a server 
 *  listening on TCP port 9764.  All data sent to a connection on 
 *  this port will be echoed back to the sender.  By default, this 
 *  demo supports 3 simultaneous connections.
 *
 *********************************************************************
 * FileName:        BerkeleyTCPServerDemo.c
 * Company:         Microchip Technology, Inc.
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
 * Author               Date    	Comment
 *~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Aseem Swalah         4/21/08  	Original
 ********************************************************************/
 
#include "TCPIPConfig.h" 

#if defined(STACK_USE_BERKELEY_API)

#include "TCPIP Stack/TCPIP.h"


#define PORTNUM 9764
#define MAX_CLIENT (3) // Maximum number of simultanous connections accepted by the server.

#define MAX_LENGTH 180

/*********************************************************************
 * Function:        void BerkeleyTCPServerDemo(void)
 *
 * PreCondition:    Stack is initialized
 *
 * Input:           char* str: pointer to the input-output buffer. int length: the lenght of the OUTPUT. If 0, no output
 *
 * Output:          the output lenght
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************/
int BerkeleyTCPServerDemo(char *str,int out_lenght){

    //Here STATIS is mandatory, since we do not want to re-init these variables each time the function is called!
    static SOCKET bsdServerSocket;   
    static SOCKET ClientSock[MAX_CLIENT];
    static BOOL ClientStatus[MAX_CLIENT] = { FALSE };
    struct sockaddr_in addr;
    struct sockaddr_in addRemote;
    int addrlen = sizeof(struct sockaddr_in);
    char bfr[MAX_LENGTH];
    int length=0;
    int i;
    static int nConnected=0;
    static enum
    {
	BSD_INIT = 0,
        BSD_CREATE_SOCKET,
        BSD_BIND,
        BSD_LISTEN,
        BSD_OPERATION
    } BSDServerState = BSD_INIT;

    switch(BSDServerState)
    {
	    case BSD_INIT:
 #if defined(PIC32_STARTER_KIT)
                DBPRINTF("INIT BERKELEY TCP SERVER\n");
#endif
        	// Initialize all client socket handles so that we don't process 
        	// them in the BSD_OPERATION state
        	for(i = 0; i < MAX_CLIENT; i++)
        		ClientSock[i] = INVALID_SOCKET;
        		
        	BSDServerState = BSD_CREATE_SOCKET;
                LED0_IO=0; //use this LED to show if there is at least 1 client connectec
        	// No break needed
	    
        case BSD_CREATE_SOCKET:
            // Create a socket for this server to listen and accept connections on
            bsdServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if(bsdServerSocket == INVALID_SOCKET)
                return length;
            
            BSDServerState = BSD_BIND;
            // No break needed

        case BSD_BIND:
            // Bind socket to a local port
            addr.sin_port = PORTNUM;
            addr.sin_addr.S_un.S_addr = IP_ADDR_ANY;
            if( bind( bsdServerSocket, (struct sockaddr*)&addr, addrlen ) == SOCKET_ERROR )
                return length;
            
            BSDServerState = BSD_LISTEN;
            // No break needed
         
      case BSD_LISTEN:
            if(listen(bsdServerSocket, MAX_CLIENT) == 0)
	            BSDServerState = BSD_OPERATION;

			// No break.  If listen() returns SOCKET_ERROR it could be because 
			// MAX_CLIENT is set to too large of a backlog than can be handled 
			// by the underlying TCP socket count (TCP_PURPOSE_BERKELEY_SERVER 
			// type sockets in TCPIPConfig.h).  However, in this case, it is 
			// possible that some of the backlog is still handleable, in which 
			// case we should try to accept() connections anyway and proceed 
			// with normal operation.
         
      case BSD_OPERATION:
            for(i=0; i<MAX_CLIENT; i++)
            {
	            // Accept any pending connection requests, assuming we have a place to store the socket descriptor
                if(ClientSock[i] == INVALID_SOCKET){
                    ClientSock[i] = accept(bsdServerSocket, (struct sockaddr*)&addRemote, &addrlen);
                
                   
                }
                // If this socket is not connected then no need to process anything
                if(ClientSock[i] == INVALID_SOCKET) {
                    continue;
                }
                else { //here we enter if the above socket is actually connected. We enter here AT EACH LOOP!

                    if (ClientStatus[i]==FALSE){
                     ClientStatus[i]=TRUE;
                     nConnected++;
#if defined (PIC32_STARTER_KIT)
                     DBPRINTF("CLIENT %i CONNECTED (Total: %i)\n",i,nConnected);
#endif
                    }
                }
	            
                //if we have something to transmit, do so!
                if (out_lenght!=0){
                  send(ClientSock[i], str, strlen(str), 0);
                }

                else{
                 length = recv( ClientSock[i], bfr, sizeof(bfr), 0);

                 if( length > 0 )
                {
                  //  I2CTransmitMoreBytes(length-1,bfr);
                    
                    bfr[length] = '\0';
//#if defined(PIC32_STARTER_KIT)
//                DBPRINTF("%i: %c -- %c\n",length,bfr[length-2],bfr[length-1]);
//#endif
                    strcpy(str,bfr);

                    if (str[length-1] == '\n' ){ //workaround from input from keyboard
                        str[length-1] = '\0'; 
                        length--;
                    }
                    //send(ClientSock[i], bfr, strlen(bfr), 0);
                }
                else if( length < 0 )
                {
                    closesocket( ClientSock[i] );                  
                    ClientSock[i] = INVALID_SOCKET;
                    
                    ClientStatus[i]=FALSE;
                    nConnected--;
#if defined (PIC32_STARTER_KIT)
                     DBPRINTF("CLIENT %i DISCONNECTED (%i connected)\n",i,nConnected);
#endif
                    }
                }
            }
            break;
         
        default:
            return length;
    }


    if (nConnected>0) LED0_IO=1;
    else LED0_IO=0;


    return length;
}

#endif //#if defined(STACK_USE_BERKELEY_API)


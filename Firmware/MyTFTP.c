#include "TCPIPConfig.h"
#include "TCPIP Stack/TCPIP.h"

#include "MyTFTP.h"
#include <stdlib.h>



static char *vDOWNLOADRemoteHost; /*The hostname, i.e. pointer to char* */
static char *vDOWNLOADFilename; /*The filename, i.e. pointer to char*  */
static IP_ADDR ipRemote; /*The server IP address*/
CHAR smDOWNLOAD = TFTP_DOWNLOAD_RESOLVE_IP; /*state machine status*/


CHAR MyTFTPClient(char *vFilename,IP_ADDR vipRemote,char *Data,int *n){

//      vDOWNLOADRemoteHost=vRemoteHost; 
	volatile TFTP_RESULT result;

	switch(smDOWNLOAD)
	{
#ifdef USE_DNS
	case TFTP_DOWNLOAD_GET_DNS:
		if(!DNSBeginUsage())
			break;
		DNSResolveROM(vDOWNLOADRemoteHost, DNS_TYPE_A);
		smDOWNLOAD = TFTP_DOWNLOAD_RESOLVE_HOST;
		break;

	case TFTP_DOWNLOAD_RESOLVE_HOST:
		if(!DNSIsResolved(&ipRemote))
			break;
		DNSEndUsage();
		if(ipRemote.Val == 0u)
		{
			smDOWNLOAD = TFTP_DOWNLOAD_HOST_RESOLVE_TIMEOUT;
			break;
		}
		TFTPOpen(&ipRemote);
		smDOWNLOAD = TFTP_DOWNLOAD_CONNECT;
		break;
#endif
        case TFTP_DOWNLOAD_RESOLVE_IP:
                ipRemote=vipRemote;          
                TFTPOpen(&ipRemote);
		smDOWNLOAD = TFTP_DOWNLOAD_CONNECT;
                break;
	case TFTP_DOWNLOAD_CONNECT:
                vDOWNLOADFilename=vFilename;
                result=TFTPIsOpened();
              	switch(result)
		{
		case TFTP_OK:
			TFTPOpenFile(vDOWNLOADFilename, TFTP_FILE_MODE_READ); /*vDOWNLOADFilename: BYTE* with filename*/
			smDOWNLOAD = TFTP_DOWNLOAD_GET_FILENAME;
			break;
		case TFTP_TIMEOUT:
			smDOWNLOAD = TFTP_DOWNLOAD_CONNECT_TIMEOUT;
			break;
		default:
			break;
		}
		break;
	case TFTP_DOWNLOAD_GET_FILENAME:
                result = TFTPIsFileOpened();
		switch(result)
		{
		case TFTP_OK:
			smDOWNLOAD = TFTP_DOWNLOAD_GET_DATA;
			break;
		case TFTP_RETRY:
			TFTPOpenROMFile(vDOWNLOADFilename, TFTP_FILE_MODE_READ);
			break;
		case TFTP_TIMEOUT:
			smDOWNLOAD = TFTP_DOWNLOAD_CONNECT_TIMEOUT;
			break;
		case TFTP_ERROR:
			smDOWNLOAD = TFTP_DOWNLOAD_SERVER_ERROR;
			break;
		default:
			break;
		}
		if(result != TFTP_OK) break;
		// No break when TFTPIsFileOpened() returns TFTP_OK -- we need to immediately start getting data
	case TFTP_DOWNLOAD_GET_DATA:
            
            while(1){
                result=TFTPIsGetReady();
                if (result==TFTP_OK){
                    //READ!
                    Data[(*n)++]=TFTPGet();
                  }
                else if (result==TFTP_END_OF_FILE){
                    DBPRINTF("CLOSE_FILE\n");
                    TFTPCloseFile();
                    smDOWNLOAD=TFTP_DOWNLOAD_WAIT_FOR_CLOSURE;
                    break; //break the while(1)
                }
                else if (result==TFTP_NOT_READY){
                    DBPRINTF("NOT READY %i\n",result);
                    break;
                }
                else {
                    DBPRINTF("BOH %i\n",result);
                    break; //break the while(1)
                }
            }
            break;
	case TFTP_DOWNLOAD_WAIT_FOR_CLOSURE:
                result=TFTPIsFileClosed();
                switch(result)
		{
		case TFTP_OK:
                case TFTP_END_OF_FILE:
			smDOWNLOAD = TFTP_DOWNLOAD_COMPLETE;
			UDPClose(_tftpSocket);
			break; /*This is the GOOD return case*/
		case TFTP_TIMEOUT:
		case TFTP_ERROR:
			smDOWNLOAD = TFTP_DOWNLOAD_SERVER_ERROR;
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}

	return smDOWNLOAD;
}

#ifdef USE_DNS
CHAR MyTFTPClientWithHostname(){
    smDOWNLOAD = TFTP_DOWNLOAD_RESOLVE_HOST;
    return MyTFTPClient();
}
#endif


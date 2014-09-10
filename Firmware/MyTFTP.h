/* 
 * File:   MyTFTP.h
 * Author: celentan
 *
 * Created on December 27, 2013, 11:47 AM
 */

#ifndef MYTFTP_H
#define	MYTFTP_H

#ifdef	__cplusplus
extern "C" {
#endif

#define TFTP_DOWNLOAD_COMPLETE				0
#define TFTP_DOWNLOAD_GET_DNS				1
#define TFTP_DOWNLOAD_RESOLVE_HOST			2
#define TFTP_DOWNLOAD_RESOLVE_IP                        3
#define TFTP_DOWNLOAD_CONNECT				4
#define TFTP_DOWNLOAD_GET_FILENAME			5
#define TFTP_DOWNLOAD_GET_DATA				6
#define TFTP_DOWNLOAD_WAIT_FOR_CLOSURE		        7
#define TFTP_DOWNLOAD_HOST_RESOLVE_TIMEOUT	        -1
#define TFTP_DOWNLOAD_CONNECT_TIMEOUT			-2
#define TFTP_DOWNLOAD_SERVER_ERROR			-3

 
#define MAX_SEQ_DATA 16384
typedef enum TFTP_Type{
    DO_NONE=0,
    DO_LED_DATA,
    DO_LED_SEQUENCE
}TFTP_Type;
CHAR MyTFTPClient(char *vFilename,IP_ADDR vipRemote,char *Data,int *n);


#ifdef	__cplusplus
}
#endif

#endif	/* MYTFTP_H */


/*
 *	File Client0.c
 *      ECHO TCP CLIENT with the following feqatures:
 *      - Gets server IP address and port from keyboard 
 *      - LINE/ORIENTED:
 *        > continuously reads lines from keyboard
 *        > sends each line to the server
 *        > waits for response and diaplays it
 *      - Terminates when the "close" or "stop" line is entered
 */


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <rpc/xdr.h>

#include <string.h>
#include <time.h>
#include <unistd.h>

#include "errlib.h"
#include "sockwrap.h"

#define BUFLEN	128
#define TOUT	10
#define INVALID_SOCKET	-1
typedef int SOCKET;

char *prog_name;
/* FUNCTION PROTOTYPES */

void showAddr(char *str, struct sockaddr_in *a);
int main(int argc,char **argv)
{
	char     	   buf[BUFLEN];		/* transmission buffer */
	char	 	   rbuf[BUFLEN];	/* reception buffer */

	uint32_t	   taddr_n;		/* server IP addr. (network byte order) */
	uint16_t	   tport_n, tport_h;	/* server port number (net/host byte order resp.) */

	SOCKET		   s;
	int		   result;
	struct sockaddr_in saddr;		/* server address structure */
	struct in_addr	   sIPaddr; 		/* server IP addr. structure */
        
         /* for errlib to know the program name */
	prog_name = argv[0];
        if(argc!=3)
              err_quit("usage %s  <dest_host> <dest_port>",prog_name);
	/* input IP address and port of server */
	result = inet_aton(argv[1], &sIPaddr);
	if (!result)
		err_ret("(%s)error-Invalid address",prog_name);

        //host port number
        tport_h=atoi(argv[2]);
  	tport_n = htons(tport_h);

	/* create the socket */
    	 err_msg("(%s) socket created",prog_name);
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		err_ret("(%s)socket() failed",prog_name);
        err_msg("(%s) done.Socket fd number:%d",prog_name,s);
	//printf("done. Socket fd number: %d\n",s);

	/* prepare address structure */
    	saddr.sin_family = AF_INET;
	saddr.sin_port   = tport_n;
	saddr.sin_addr   = sIPaddr;

	/* connect */
	
	result = connect(s, (struct sockaddr *) &saddr, sizeof(saddr));
	if (result == -1)
		err_ret("(%s)connect() failed",prog_name);
	showAddr("Connecting to target address", &saddr);
       
        while(1){
       int len,nread;
        /*-------send to the server-------*/
        err_msg("(%s)Please input two integers:",prog_name);
        fgets(buf,BUFLEN,stdin);
        len=strlen(buf);
        buf[strlen(buf)-1]='\0';
        strcat(buf,"\r\n");
        Writen(s,buf,strlen(buf));
        /*--------read from the server-------*/
         nread=readline_unbuffered(s,rbuf,BUFLEN);
         if(nread==0)
           return 0;
         else if(nread<0)
         {
          err_ret ("(%s) error - readline() failed", prog_name);
          return 0;
         }
         printf("%s",rbuf);
        }
	
	close(s);
	exit(0);
}

void showAddr(char *str, struct sockaddr_in *a)
{
    char *p;
    
    p = inet_ntoa(a->sin_addr);
	err_msg("(%s)%s %s!%u",prog_name,str,p,ntohs(a->sin_port));
}


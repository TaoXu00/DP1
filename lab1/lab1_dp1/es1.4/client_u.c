/*
 *	File Client_u.c
 *      ECHO UDP CLIENT with the following feqatures:
 *      - Gets server IP address and port from keyboard 
 *      - LINE/ORIENTED:
 *        > continuously reads lines from keyboard
 *        > sends each line to the server
 *        > waits for response and diaplays it    
          > waits for response (at most for a fixed amount of time) and diaplays it
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

#define BUFLEN	32
#define TIMEOUT 15
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
	struct sockaddr_in saddr,caddr;		/* server address structure */
//	struct in_addr	   sIPaddr; 		/* server IP addr. structure */
        fd_set		   cset;
	struct timeval	   tval;
   	int 		  result, namelen;

         /* for errlib to know the program name */
        prog_name = argv[0];
        if(argc!=4)
              err_quit("usage %s  <dest_host> <dest_port> <message>",prog_name);
       
	/* input IP address and port of server */
	taddr_n = inet_addr(argv[1]);
	if (taddr_n == INADDR_NONE)
		err_ret("(%s)error-Invalid address",prog_name);
        //host port number
        tport_h=atoi(argv[2]);
  	tport_n = htons(tport_h);

	/* create the socket */
    	 err_msg("(%s) socket created",prog_name);
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
		err_ret("(%s)socket() failed",prog_name);
        //err_msg("(%s) done.Socket fd number:%d",prog_name,s);
	//printf("done. Socket fd number: %d\n",s);

	/* prepare client address structure */
    	caddr.sin_family = AF_INET;
	caddr.sin_port   = htons(0);
	caddr.sin_addr.s_addr   = htonl(INADDR_ANY);

        /* bind */
	//err_msg("(%s)Binding to an unused port number",prog_name);
	result = bind(s, (struct sockaddr *) &caddr, sizeof(caddr) );
	if (result == -1)
		err_ret("(%s)bind() failed",prog_name);
	namelen = sizeof(caddr);
	getsockname(s, (struct sockaddr *) &caddr, &namelen );
	//showAddr("done. Bound to addr: ", &caddr);
        
        /* prepare server address structure */
    	saddr.sin_family      = AF_INET;
	saddr.sin_port        = tport_n;
	saddr.sin_addr.s_addr = taddr_n;
        showAddr("destination ", &saddr);
       
       /*-------------start to transmit the msg to server----------*/
	    int			len, n;
	    struct sockaddr_in 	from;
	    int 		fromlen;
            
             
	    sprintf(buf,"%s",argv[3]);
	    len = strlen(buf);
	    n=sendto(s, buf, len, 0, (struct sockaddr *) &saddr, sizeof(saddr));
	    if (n != len)
		err_quit("(%s)Write error",prog_name);
	    err_msg("(%s)-data has been sent",prog_name);
            FD_ZERO(&cset);
	    FD_SET(s, &cset);
	    tval.tv_sec = TIMEOUT;
	    tval.tv_usec = 0;
            
	    n = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
	    if (n == -1)
		err_quit("(%s)select() failed",prog_name);
             
	    if (n>0)
            {  
		/* receive datagram */
		fromlen = sizeof(struct sockaddr_in);
	    	n=recvfrom(s,rbuf,BUFLEN-1,0,(struct sockaddr *)&from,&fromlen);
                if (n != -1)
	    	{
			rbuf[n] = '\0';			
			err_msg("(%s)---received string %s",prog_name,rbuf);
                   
	    	}
		else err_quit("(%s)Error in receiving response",prog_name);
	    }
	    else err_quit("(%s)Timeout expired",prog_name);
	    printf("Received:%s\n",rbuf);
	
	close(s);
	exit(0);
}

void showAddr(char *str, struct sockaddr_in *a)
{
    char *p;
    
    p = inet_ntoa(a->sin_addr);
	err_msg("(%s)%s %s!%u",prog_name,str,p,ntohs(a->sin_port));
}


/*
 * 	File Server0u.c
 *	ECHO UDP SERVER with the following features:
 *      - Gets port from keyboard 
 *      - SEQUENTIAL: serves one request at a time
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
#include    "sockwrap.h"
#include    "errlib.h"

#define BUFLEN		65536
#define INVALID_SOCKET	-1
typedef int SOCKET;

char *prog_name;
/* FUNCTION PROTOTYPES */
void showAddr(char *str, struct sockaddr_in *a);
int main(int argc,char **argv)
{
	uint16_t 		lport_n, lport_h;	/* port where server listens */
	SOCKET			s;
	int			result, addrlen, n;
	struct sockaddr_in	saddr, from;

	prog_name=argv[0];
	/* input server port number */
        if(argc!=2)
         err_quit("usage: %s <port>",prog_name);
	lport_h=atoi(argv[1]);
  	lport_n = htons(lport_h);

	/* create the socket */
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
		err_quit("(%s)socket() failed",prog_name);
	err_msg("(%s)socket created",prog_name);

	/* bind the socket to all local IP addresses */
	saddr.sin_family      = AF_INET;
	saddr.sin_port        = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;
	showAddr("listening for UDP packets on ", &saddr);
	result = bind(s, (struct sockaddr *) &saddr, sizeof(saddr));
	if (result == -1)
		err_quit("(%s)bind() failed",prog_name);
	

	/* main server loop */
	for (;;)
	{
            err_msg("(%s)waiting for a packet...",prog_name);
	    addrlen = sizeof(struct sockaddr_in);
	    n=recvfrom(s, buf, BUFLEN-1, 0, (struct sockaddr *)&from, &addrlen);
	    if (n != -1)
	    {
	    	buf[n] = '\0';
	    	err_msg("(%s)---received string '%s' (payloads larger than 255 bytes are truncated)",prog_name,buf);
	    	if(sendto(s, buf, n, 0, (struct sockaddr *)&from, addrlen) != n)
			err_quit("(%s)Write error while replying",prog_name);
	        else
			err_msg("(%s)Reply sent",prog_name);
	    }
	}
}

void showAddr(char *str, struct sockaddr_in *a)
{
    char *p;
    
    p = inet_ntoa(a->sin_addr);
	err_msg("(%s)%s %s!%u",prog_name,str,p,ntohs(a->sin_port));
}

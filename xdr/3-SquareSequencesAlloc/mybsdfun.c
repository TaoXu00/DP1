/*	UTILITY FUNCTIONS FOR IPV4 SOCKET APPLICATIONS
        UNIX BSD4.3 VERSION
*/


#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


void err_fatal(char *mes);
void SockStartup(void);
void SockCleanup(void);
void showAddr(char * str, struct sockaddr_in * a);

/* Error handling function for fatal errors */
void err_fatal(char *mes)
{
	printf("%s, errno=%d\n", mes, errno);
	perror("");
	exit(1);
}

/* Error handling function for nonfatal errors */
void err_continue(char *mes)
{
	printf("%s, errno=%d\n", mes, errno);
	perror("");
}

/* BSD4.3 Sockets Api: Initialization function */
void SockStartup(void)
{
	/* don't need initialization */
        return;
}

/* BSD4.3 Sockets Api: Cleanup function */
void SockCleanup(void)
{
        /* don't need cleanup */
        return;
}

/* Utility function to display a string str
   followed by an internet address a, written
   in dotted decimal notation
*/
void showAddr(char *str, struct sockaddr_in *a)
{
	uint32_t ipaddr;
        unsigned char *p;

	ipaddr  = a->sin_addr.s_addr;
	p 	= (unsigned char *) &ipaddr;
	printf("%s ",str);
	printf("%03d.", p[0] );
	printf("%03d.", p[1] );
	printf("%03d.", p[2] );
	printf("%03d!", p[3] );
	printf("%u\n", ntohs(a->sin_port));
}

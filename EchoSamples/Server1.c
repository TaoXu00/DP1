 /*
 * 	File Server1.c
 *	ECHO TCP SERVER with the following features:
 *      - Gets port from keyboard 
 *      - SIMULATED CONCURRENCY: single process single thread but with the possibility
 *        to have more than one client with service in progress
 *       
 */


#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include     <inttypes.h>
#include    "mysocket.h"

#define DBG_FLAG	0	/* define 1 to enable debug info, 0 to disable it */
#define RBUFLEN		128

/* GLOBAL VARIABLES */
fd_set	ractive, wactive;		/* sets of active sockets */
typedef struct {
	unsigned nlefttowrite;
	unsigned char *ptr;
	char buf[RBUFLEN];
} Sockdata;
Sockdata sdata[FD_SETSIZE];	/* client status information */

/* FUNCTION PROTOTYPES */
int mygetline(char * line, size_t maxline, char *prompt);
void printMask(char * str, fd_set *m);
void shut_down_rcvr(void);
SOCKET getNextAvailSkt(fd_set *rset, fd_set *wset);

main()
{
	SOCKET			conn_request_skt; /* socket where connections are accepted */
	char			buf[RBUFLEN];	  /* reception buffer */
	uint16_t 		lport_n, lport_h; /* port where the server listens (net/host byte order resp.) */
	int			bklog = 2;	  /* listen backlog */
	SOCKET			s;
	int			result, addrlen;
	struct sockaddr_in	saddr, c_addr;    /* server and client address structures */ 

	/* Initialize socket API if needed */
	SockStartup();

	/* input server port number */
	mygetline(buf, RBUFLEN, "Enter port : ");
	if (sscanf(buf, "%" SCNu16, &lport_h)!=1)
		err_fatal("Invalid port number");
  	lport_n = htons(lport_h);

	/* create the socket */
	printf("creating socket\n");
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		err_fatal("socket() failed");
	printf("done, socket number %u\n",s);

	/* bind the socket to any local IP address */
	saddr.sin_family      = AF_INET;
	saddr.sin_port        = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;
	showAddr("Binding to address", &saddr);
	result = bind(s, (struct sockaddr *) &saddr, sizeof(saddr));
	if (result == -1)
		err_fatal("bind() failed");
	printf("done.\n");

	/* listen */
	printf ("Listening at socket %d with backlog = %d \n",s,bklog);
	result = listen(s, bklog);
	if (result == -1)
		err_fatal("listen() failed");
	printf("done.\n");

	/* save s as the socket for accepting connections */
	conn_request_skt = s;
	/* and initialize active sets (empty write set and only s in the read set) */
	FD_ZERO(&ractive);
	FD_SET(conn_request_skt, &ractive);
	FD_ZERO(&wactive);
	printf("This server can serve concurrently up to %d clients\n", FD_SETSIZE);

	/* main server loop */
	for (;;)
	{
		fd_set	rready, wready;	/* sets of ready sockets */

		/* select active sockets */
		memcpy((char *) &rready, (char *) &ractive, sizeof(fd_set));
		memcpy((char *) &wready, (char *) &wactive, sizeof(fd_set));
		if (DBG_FLAG) printMask("\nread mask before select ", &rready);
		if (DBG_FLAG) printMask("\nwrite mask before select ", &wready);
		result = select(FD_SETSIZE, &rready, &wready, NULL, NULL);
		if (result == -1) {
			err_continue("select() failed");
			shut_down_rcvr();
		}
		if (DBG_FLAG) printMask("\nread mask after select ", &rready);
		if (DBG_FLAG) printMask("\nwrite mask after select ", &wready);

		/* test on selected sockets */
		if (FD_ISSET(conn_request_skt, &rready))
		{
			/* passive socket ready for accepting */
			addrlen = sizeof(struct sockaddr_in);
			s = accept(conn_request_skt, (struct sockaddr *) &c_addr, &addrlen);
			if (s == INVALID_SOCKET)
				err_continue("accept() failed");
			else {
				showAddr("Accepted connection from", &c_addr);
				printf("new socket: %u\n",s);
				FD_SET(s, &ractive);
				sdata[s].ptr=sdata[s].buf;
				sdata[s].nlefttowrite=0;
			}
		}
		else
		{
			int	n;

			s = getNextAvailSkt(&rready, &wready);
			if (s == INVALID_SOCKET) continue;
			if (FD_ISSET(s, &rready)) {
			    /* socket s ready for reading */
			    n=recv(s, sdata[s].buf, RBUFLEN-1, 0);
	        	    if (n < 0)
			    {
		    	    	printf("Read error\n");
			    	FD_CLR(s, &ractive);
			    	closesocket(s);
			    	printf("Socket %d closed\n", s);
			    }
	        	    else if (n == 0)
			    {
		    	    	printf("Connection closed by party on socket %d\n", s);
			    	FD_CLR(s, &ractive);
			    	closesocket(s);
			    }
	        	    else
			    {
			    	if (DBG_FLAG) printf("Received data from socket %03d : \n",s);
				sdata[s].buf[n]=0;
			    	printf("[%s]\n",sdata[s].buf);
			    	FD_CLR(s, &ractive); FD_SET(s, &wactive);
			    	sdata[s].nlefttowrite=n;
			    }
			}
			else {
			    /* socket s ready for writing */
			    n = send(s, sdata[s].ptr, sdata[s].nlefttowrite, 0);
			    if (n <=0) 
			    {
				printf("Write error while replying on socket %d\n", s);
				FD_CLR(s, &wactive);
				closesocket(s);
			    	printf("Socket %d closed\n", s);
			    }
			    else
			    {
				sdata[s].nlefttowrite -= n;
				sdata[s].ptr += n;
				if (sdata[s].nlefttowrite==0) {
				    if (DBG_FLAG) printf("Reply data sent on socket %d\n", s);
				    FD_CLR(s, &wactive); FD_SET(s, &ractive);
				    sdata[s].ptr=sdata[s].buf;
				}
			    }
			}
		}
	}
}


/* Shuts down the server */
void shut_down_rcvr(void)
{
	SOCKET	s;

	printf("Closing sockets [ ");
	while ((s = getNextAvailSkt(&ractive, &wactive)) != INVALID_SOCKET)
	{
		printf("%u ", s);
		closesocket(s);
		if (FD_ISSET(s, &ractive)) FD_CLR(s, &ractive);
		if (FD_ISSET(s, &wactive)) FD_CLR(s, &wactive);
	}
	printf("], \n");
	SockCleanup();
	printf("Server shutdown completed\n");
	exit(0);
}


/* Prints a set of sockets (m) preceded by a string (str) */
void printMask(char *str, fd_set *m)
{
	int   	i;

	printf("%-17s : [", str);
	for (i = 0; i < FD_SETSIZE; i++)
	{
		if (i % 8 == 0)
			putchar(' ');
		if (FD_ISSET(i, m))
			putchar('1');
		else
			putchar('0');
	}
	printf(" ]  ");
}

/* Looks for the next available socket in fd_sets rset and wset */
SOCKET getNextAvailSkt(fd_set *rset, fd_set *wset)
{
	static	SOCKET	s = 0;
	int		i;

	s %= FD_SETSIZE;
	for (i = s; i < FD_SETSIZE; i++)
		if (FD_ISSET(i, rset) || FD_ISSET(i, wset))
		{
			s = i+1;
			return(i);
		}
	for (i = 0; i < s; i++)
		if (FD_ISSET(i, rset) || FD_ISSET(i, wset))
		{
			s = i+1;
			return(i);
		}
	return(INVALID_SOCKET);
}

/* Gets a line of text from standard input after having printed a prompt string 
   Substitutes end of line with '\0'
   Empties standard input buffer but stores at most maxline-1 characters in the
   passed buffer
*/
int mygetline(char *line, size_t maxline, char *prompt)
{
	char	ch;
	size_t  i;

	printf("%s", prompt);
	for (i=0; i< maxline-1 && (ch = getchar()) != '\n' && ch != EOF; i++)
		*line++ = ch;
	*line = '\0';
	while (ch != '\n' && ch != EOF)
		ch = getchar();
	if (ch == EOF)
		return(EOF);
	else    return(1);
}


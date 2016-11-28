/*
 * 	File Server0.c
 *	ECHO TCP SERVER with the following features:
 *      - Gets port from keyboard 
 *      - SEQUENTIAL: serves one client at a time
 */

#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <inttypes.h>
#include    "mysocket.h"

#define RBUFLEN		128

/* FUNCTION PROTOTYPES */
int mygetline(char * line, size_t maxline, char *prompt);
int writen(SOCKET, char *, size_t);

main()
{
	SOCKET	 		conn_request_skt;	/* socket where connections are accepted */
	char	 		buf[RBUFLEN];		/* reception buffer */
	uint16_t 		lport_n, lport_h;	/* port where the server listens (net/host byte ord resp.) */
	int	 		bklog = 2;		/* listen backlog */
	SOCKET	 		s;			
	int	 		result, addrlen, n;
	struct sockaddr_in 	saddr, caddr;		/* server and client address structures */ 

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

	conn_request_skt = s;

	/* main server loop */
	for (;;)
	{
		/* accept next connection */
		addrlen = sizeof(struct sockaddr_in);
		s = accept(conn_request_skt, (struct sockaddr *) &caddr, &addrlen);
		if (s == INVALID_SOCKET)
			err_fatal("accept() failed");
		showAddr("Accepted connection from", &caddr);
		printf("new socket: %u\n",s);

		/* serve the client on socket s */
		for (;;)
		{
		    n=recv(s, buf, RBUFLEN-1, 0);
	            if (n < 0)
		    {
		       printf("Read error\n");
		       closesocket(s);
		       printf("Socket %d closed\n", s);
		       break;
		    }
		    else if (n==0)
		    {
		       printf("Connection closed by party on socket %d\n",s);
		       closesocket(s);
		       break;
		    }
	            else
		    {
		       printf("Received line from socket %03d :\n", s);
		       buf[n]=0;
		       printf("[%s]\n",buf);
		       if(writen(s, buf, n) != n)
		    	   printf("Write error while replying\n");
		       else
		    	   printf("Reply sent\n");
		    }	
		}
	}
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

/* Writes nbytes from buffer ptr to stream socket s */
int writen(SOCKET s, char *ptr, size_t nbytes)
{
    size_t nleft;
    ssize_t nwritten;

    for (nleft=nbytes; nleft > 0; )
    {
	nwritten = send(s, ptr, nleft, 0);
	if (nwritten <=0)
	    return (nwritten);
	else
	{
	    nleft -= nwritten;
	    ptr += nwritten;   
	}
    }
    return (nbytes - nleft);
}

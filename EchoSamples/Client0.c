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


#include     <stdio.h>
#include     <stdlib.h>
#include     <string.h>
#include     <inttypes.h>
#include     "mysocket.h"

#define BUFLEN	128
#define TOUT	10

/* FUNCTION PROTOTYPES */
int mygetline(char * line, size_t maxline, char *prompt);
int writen(SOCKET, char *, size_t);
int readline (SOCKET s, char *ptr, size_t maxlen);
int iscloseorstop(char *buf);

main()
{
	char     	   buf[BUFLEN];		/* transmission buffer */
	char	 	   rbuf[BUFLEN];	/* reception buffer */

	uint32_t	   taddr_n;		/* server IP addr. (network byte order) */
	uint16_t	   tport_n, tport_h;	/* server port number (net/host byte order resp.) */

	SOCKET		   s;
	int		   result;
	struct sockaddr_in saddr;		/* server address structure */
	struct in_addr	   sIPaddr; 		/* server IP addr. structure */


	/* Initialize socket API if needed */
	SockStartup();

	/* input IP address and port of server */
	mygetline(buf, BUFLEN, "Enter host IPv4 address (dotted notation):");
	result = inet_aton(buf, &sIPaddr);
	if (!result)
		err_fatal("Invalid address");

	mygetline(buf, BUFLEN, "Enter port : ");
	if (sscanf(buf, "%" SCNu16, &tport_h)!=1)
		err_fatal("Invalid port number");
  	tport_n = htons(tport_h);

	/* create the socket */
    	printf("Creating socket\n");
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		err_fatal("socket() failed");
	printf("done. Socket fd number: %d\n",s);

	/* prepare address structure */
    	saddr.sin_family = AF_INET;
	saddr.sin_port   = tport_n;
	saddr.sin_addr   = sIPaddr;

	/* connect */
	showAddr("Connecting to target address", &saddr);
	result = connect(s, (struct sockaddr *) &saddr, sizeof(saddr));
	if (result == -1)
		err_fatal("connect() failed");
	printf("done.\n");

	/* main client loop */
	printf("Enter line 'close' or 'stop' to close connection and stop client.\n");
	
	for (buf[0]='\0' ; !iscloseorstop(buf); )
	{
	    size_t	len;

	    mygetline(buf, BUFLEN, "Enter line (max 127 char): ");
	    strcat(buf,"\n");
	    len = strlen(buf);
	    if(writen(s, buf, len) != len)
	    {
		printf("Write error\n");
		break;
	    }

	    printf("waiting for response...\n");
	    result = readline(s, rbuf, BUFLEN);
	    if (result <= 0)
	    {
		 printf("Read error/Connection closed\n");
		 closesocket(s);
		 SockCleanup();
		 exit(1);
	    }
	    else
	    {
		    rbuf[result-1] = '\0';
		    printf("Received response from socket %03u : \n[%s]\n", s, rbuf);
	    }
	    printf("===========================================================\n");
	}
	closesocket(s);
	SockCleanup();
	exit(0);
}

/* Gets a line of text from standard input after having printed a prompt string 
   Substitutes end of line with '\0'
   Empties standard input buffer but stores at most maxline-1 characters in the
   passed buffer
*/
int mygetline(char *line, size_t maxline, char *prompt)
{
	char	ch;
	size_t 	i;

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

/* Reads a line from stream socket s to buffer ptr 
   The line is stored in ptr including the final '\n'
   At most maxlen chasracters are read
*/
int readline (SOCKET s, char *ptr, size_t maxlen)
{
    size_t n;
    ssize_t nread;
    char c;

    for (n=1; n<maxlen; n++)
    {
	nread=recv(s, &c, 1, 0);
	if (nread == 1)
	{
	    *ptr++ = c;
	    if (c == '\n') 
		break;
	}
	else if (nread == 0)	/* connection closed by party */
	{
	    *ptr = 0;
	    return (n-1);
	}
	else 			/* error */
	    return (-1);
    }
    *ptr = 0;
    return (n);
}


/* Writes nbytes from buffer ptr to stream socket s */
int writen(SOCKET s, char *ptr, size_t nbytes)
{
    size_t  nleft;
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

/* Checks if the content of buffer buf equals the "close" o "stop" line */
int iscloseorstop(char *buf)
{
	return (!strcmp(buf, "close\n") || !strcmp(buf, "stop\n"));
}

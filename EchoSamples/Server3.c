/*
 * 	File Server3.C
 *	ECHO TCP SERVER with the following features:
 *      - Gets port from keyboard 
 *      - CONCURRENT SERVER based on UNIX process pooling
 */


#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <inttypes.h>
#include    "mysocket.h"

#define RBUFLEN		128
#define NPROCS		5	/* number of processes in pool */

/* GLOBAL VARIABLES */
char buf[RBUFLEN];		/* reception buffer */

/* Prototipi delle funzioni */
/* FUNCTION PROTOTYPES */
int mygetline(char * line, size_t maxline, char *prompt);
int writen(SOCKET, char *, size_t);
void service(SOCKET);
void pool_proc(SOCKET);

main()
{
	SOCKET			conn_request_skt; /* socket where connections are accepted */
	uint16_t 		lport_n, lport_h; /* port where the server listens (net/host byte order resp.) */
	int			bklog = 2;	  /* listen backlog */
	int			childpid;
	SOCKET		        s;
	int			result;
	struct sockaddr_in	saddr;
	int 			i;

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

	/* start listening */
	printf ("Listening at socket %d with backlog = %d \n",s,bklog);
	result = listen(s, bklog);
	if (result == -1)
		err_fatal("listen() failed");
	printf("done.\n");

	conn_request_skt = s;
	
	/* create processes */
	for (i=0; i<NPROCS; i++)
	{
	    if((childpid=fork())<0)
		err_fatal("fork() failed");
	    else if (childpid == 0)
	    {
		/* child process */
		pool_proc(conn_request_skt);	/* enter service loop */
	    }
	}
	printf("Process pool created\n");
	
	/* the master itself behaves as a server */
	/*
	pool_proc(conn_request_skt); 
	*/	
}

/* Executes the service loop */
void pool_proc(SOCKET conn_request_skt)
{
	SOCKET		        s;
	int			addrlen;
	struct sockaddr_in	c_addr;
	
	/* main loop */
	for (;;)
	{
	    /* accept new connection */
	    addrlen = sizeof(struct sockaddr_in);
	    s = accept(conn_request_skt, (struct sockaddr *) &c_addr, &addrlen);
	    if (s == INVALID_SOCKET)
	    	err_fatal("accept() failed");
	    showAddr("Accepted connection from", &c_addr);
	    service(s);			/* serve client */
	}
}

/* Provides echo service on the passed socket */
void service(SOCKET s)
{
    int	 n;

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
	    printf("Received line from socket %03d : \n",s);
	    buf[n]=0;
	    printf("[%s]\n",buf);
	    if(writen(s, buf, n) != n)
		printf("Write error while replying\n");
	    else
		printf("Reply sent\n");
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

/*
 *	File Client1u.C
 *      ECHO UDP CLIENT with the following feqatures:
 *      - Gets server IP address and port from keyboard 
 *      - LINE/ORIENTED:
 *        > continuously reads lines from keyboard
 *        > sends each line to the server
 *        > waits for response (at most for a fixed amount of time) and diaplays it
 *      - Terminates when the "close" or "stop" line is entered
 */


#include     <stdio.h>
#include     <stdlib.h>
#include     <string.h>
#include     <inttypes.h>
#include     "mysocket.h"

#define BUFLEN 128
#define TIMEOUT 15

/* FUNCTION PROTOTYPES */
int mygetline(char *line, size_t maxline, char *prompt);
int iscloseorstop(char *buf);

main()
{
	char     		buf[BUFLEN];	   /* transmission buffer */
	char	 		rbuf[BUFLEN];	   /* reception buffer */

	uint32_t		taddr_n, taddr_h;  /* server IP addr. (net/host byte order resp.) */
	uint16_t		tport_n, tport_h;  /* server port number */

	SOCKET			s;
	struct sockaddr_in	saddr, caddr;
	fd_set			cset;
	struct timeval		tval;
   	int 			result, namelen;

	/* Initialize socket API if needed */
	SockStartup();

	/* input IP address and port of server */
	mygetline(buf, BUFLEN, "Enter host address (dotted notation):");
	taddr_n = inet_addr(buf);
	if (taddr_n == INADDR_NONE)
		err_fatal("Invalid address");
	taddr_h = ntohl(taddr_n);

	mygetline(buf, BUFLEN, "Enter port : ");
	if (sscanf(buf, "%" SCNu16, &tport_h)!=1)
		err_fatal("Invalid port number");
  	tport_n = htons(tport_h);

	/* create the socket */
    	printf("Creating socket\n");
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
		err_fatal("socket() failed");
	printf("done. Socket number: %d\n",s);

	/* prepare client address structure */
    	caddr.sin_family      = AF_INET;
	caddr.sin_port        = htons(0);  /* ask bind for an unused port number */
	caddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/* bind */
	printf("Binding to an unused port number\n");
	result = bind(s, (struct sockaddr *) &caddr, sizeof(caddr) );
	if (result == -1)
		err_fatal("bind() failed");
	namelen = sizeof(caddr);
	getsockname(s, (struct sockaddr *) &caddr, &namelen );
	showAddr("done. Bound to addr: ", &caddr);

	/* prepare server address structure */
    	saddr.sin_family      = AF_INET;
	saddr.sin_port        = tport_n;
	saddr.sin_addr.s_addr = taddr_n;

	/* main client loop */
	printf("Use message 'close' or 'stop' to terminate program.\n");
	
	for (buf[0]='\0' ; !iscloseorstop(buf); )
	{
	    int			len, n;
	    struct sockaddr_in 	from;
	    int 		fromlen;

	    mygetline(buf, BUFLEN, "Enter msg (max 127 char): ");
	    len = strlen(buf);
	    n=sendto(s, buf, len, 0, (struct sockaddr *) &saddr, sizeof(saddr));
	    if (n != len)
	    {
		printf("Write error\n");
		continue;
	    }

	    printf("waiting for response...\n");
	    FD_ZERO(&cset);
	    FD_SET(s, &cset);
	    tval.tv_sec = TIMEOUT;
	    tval.tv_usec = 0;
	    n = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
	    if (n == -1)
		err_fatal("select() failed");
	    if (n>0)
            {
		/* receive datagram */
		fromlen = sizeof(struct sockaddr_in);
	    	n=recvfrom(s,rbuf,BUFLEN-1,0,(struct sockaddr *)&from,&fromlen);
                if (n != -1)
	    	{
			rbuf[n] = '\0';
			showAddr("Received response from", &from);
			printf(": [%s]\n", rbuf);

	    	}
		else printf("Error in receiving response\n");
	    }
	    else printf("No response received after %d seconds\n",TIMEOUT);
	    printf("=======================================================\n");
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

/* Funzione per controllare se un buffer contiene la stringa "close" o "stop" */
int iscloseorstop(char *buf)
{
	return (!strcmp(buf, "close") || !strcmp(buf, "stop"));
}

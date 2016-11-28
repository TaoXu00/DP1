/*
 * 	File Server0u.c
 *	ECHO UDP SERVER with the following features:
 *      - Gets port from keyboard 
 *      - SEQUENTIAL: serves one request at a time
 */


#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <inttypes.h>
#include    "mysocket.h"

#define BUFLEN		65536

/* FUNCTION PROTOTYPES */
int mygetline(char * line, size_t maxline, char *prompt);

main()
{
	char	 		buf[BUFLEN];		/* reception buffer */
	uint16_t 		lport_n, lport_h;	/* port where server listens */
	SOCKET			s;
	int			result, addrlen, n;
	struct sockaddr_in	saddr, from;

	/* Initialize socket API if needed */
	SockStartup();

	/* input server port number */
	mygetline(buf, BUFLEN, "Enter port : ");
	if (sscanf(buf, "%" SCNu16, &lport_h)!=1)
		err_fatal("Invalid port number");
  	lport_n = htons(lport_h);

	/* create the socket */
	printf("creating socket\n");
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
		err_fatal("socket() failed");
	printf("done, socket number %u\n",s);

	/* bind the socket to all local IP addresses */
	saddr.sin_family      = AF_INET;
	saddr.sin_port        = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;
	showAddr("Binding to address", &saddr);
	result = bind(s, (struct sockaddr *) &saddr, sizeof(saddr));
	if (result == -1)
		err_fatal("bind() failed");
	printf("done.\n");

	/* main server loop */
	for (;;)
	{
	    addrlen = sizeof(struct sockaddr_in);
	    n=recvfrom(s, buf, BUFLEN-1, 0, (struct sockaddr *)&from, &addrlen);
	    if (n != -1)
	    {
	    	buf[n] = '\0';
	    	showAddr("Received message from", &from);
	    	printf(": [%s]\n", buf);

	    	if(sendto(s, buf, n, 0, (struct sockaddr *)&from, addrlen) != n)
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


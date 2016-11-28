/*
 * Sviluppato da Enrico Masala <masala@polito.it> , Apr 2011
 * client which connects to a server, asks for a file name given from a command line and displays it
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string.h>
#include <time.h>

#include "errlib.h"
#include "sockwrap.h"

#define MAXBUFL 255
#define MAX_STR 1023

#define MSG_OK "+OK"

#ifdef TRACE
#define trace(x) x
#else
#define trace(x)
#endif

char *prog_name;



int main (int argc, char *argv[]) {

	int sockfd, err=0;
	char *dest_h, *dest_p;
	struct sockaddr_in destaddr;
	struct sockaddr_in *solvedaddr;
	int op1, op2, res, nconv;
	char buf[MAXBUFL];
	struct addrinfo *list;
	int err_getaddrinfo;
	char *fname;


	/* for errlib to know the program name */
	prog_name = argv[0];

	/* check arguments */
	if (argc!=4)
		err_quit ("usage: %s <dest_host> <dest_port> <filename>", prog_name);
	dest_h=argv[1];
	dest_p=argv[2];
	fname=argv[3];

	Getaddrinfo(dest_h, dest_p, NULL, &list);
	solvedaddr = (struct sockaddr_in *)list->ai_addr;

	/* create socket */
	sockfd = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	/* specify address to bind to */
	memset(&destaddr, 0, sizeof(destaddr));
	destaddr.sin_family = AF_INET;
	destaddr.sin_port = solvedaddr->sin_port;
	destaddr.sin_addr.s_addr = solvedaddr->sin_addr.s_addr;

	trace ( err_msg("(%s) socket created",prog_name) );

	Connect ( sockfd, (struct sockaddr *)&destaddr, sizeof(destaddr) );

	trace ( err_msg("(%s) connected to %s:%u", prog_name, inet_ntoa(destaddr.sin_addr), ntohs(destaddr.sin_port)) );


	sprintf(buf, "GET %s\r\n", fname);
	
	Write(sockfd, buf, strlen(buf));

	trace ( err_msg("(%s) - data has been sent", prog_name) );

	fd_set rset;
	struct timeval tv;
	FD_ZERO(&rset);
	FD_SET(sockfd, &rset);
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	if (select(sockfd+1, &rset, NULL, NULL, &tv) > 0) {
		int nread = 0; char c;
		do {
			// NB: do NOT use Readline since it buffers the input
			Read(sockfd, &c, sizeof(char));
			buf[nread++]=c;
		} while (c != '\n' && nread < MAXBUFL-1);
		buf[nread]='\0';

		while (nread > 0 && (buf[nread-1]=='\r' || buf[nread-1]=='\n')) {
			buf[nread-1]='\0';
			nread--;
		}
		trace( err_msg("(%s) --- received string '%s'",prog_name, buf) );

		if (nread >= strlen(MSG_OK) && strncmp(buf,MSG_OK,strlen(MSG_OK))==0) {
			char fnamestr[MAX_STR+1];
			sprintf(fnamestr, "down_%s", fname);
			int n = Read(sockfd, buf, 4);
			//printf("n=%d\n",n);
			//printf("%0x%0x%0x%0x\n",buf[0], buf[1], buf[2], buf[3]);
			uint32_t file_bytes = ntohl((*(uint32_t *)buf));
			trace( err_msg("(%s) --- received file size '%u'",prog_name, file_bytes) );

			FILE *fp;
			if ( (fp=fopen(fnamestr, "wb"))!=NULL) {
				char c;
				int i;
				for (i=0; i<file_bytes; i++) {
					Read (sockfd, &c, sizeof(char));
					fwrite(&c, sizeof(char), 1, fp);
				}
				fclose(fp);
				trace( err_msg("(%s) --- received and wrote file '%s'",prog_name, fnamestr) );
			} else {
				trace( err_msg("(%s) --- cannot open file '%s'",prog_name, fnamestr) );
			}
		} else {
			trace ( err_quit("(%s) - protocol error: received response '%s'", prog_name, buf) );
		}

	} else {
		printf("Timeout waiting for an answer from server\n");
	}
	/* At this point, the client can wait on Read , but if the server is shut down, the connection is closed and the Read returns */
	/* If the client is not waiting on Read, the server shut down does not affect the client */
	/* Read on a socket closed by the server immediately returns 0 bytes */
	/*	
	trace( err_msg("(%s) --- sleeping 5 sec",prog_name) );
	sleep(5);
	char d;
	trace( err_msg("(%s) --- reading 1 char",prog_name) );
	int nr = Read (sockfd, &d, sizeof(char));
	trace( err_msg("(%s) --- Read returned %d bytes",prog_name, nr) );
	*/	

	Close (sockfd);

	return 0;
}


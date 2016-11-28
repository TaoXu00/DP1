/*
 *	File SquareClient.c
 *      A UDP client for the Square service
 *      - Gets server IP address and port from command line, default is 127.0.0.1!2050.
 *        > continuously reads integers from keyboard
 *        > sends each integer to the server as a XDR hyper integer
 *        > waits for response (at most for a fixed amount of time) and diaplays result
 */


#include     <stdio.h>
#include     <stdlib.h>
#include     <string.h>
#include     <inttypes.h>
#include     <rpc/xdr.h>
#include     "types.h"
#include     "mysocket.h"

#define BUFLEN 512
#define TIMEOUT 15
#define DEFAULT_PORT	2050
#define DEFAULT_ADDR	"127.0.0.1"

/* FUNCTION PROTOTYPES */
SOCKET udp_client_init (struct in_addr sinaddr, uint16_t tport_n);

main(int argc, char *argv[])
{
	char     		buf[BUFLEN];	   /* transmission buffer */
	char	 		rbuf[BUFLEN];	   /* reception buffer */

	uint32_t		taddr_n, taddr_h;  /* server IP addr. (net/host byte order resp.) */
	uint16_t		tport_n, tport_h;  /* server port number */

	SOCKET			s;		   /* socket used for communication with the server */
	struct in_addr		sinaddr_def;	   /* server IP address structure (default value) */
	struct in_addr		sinaddr;	   /* server IP address structure */
	fd_set			cset;		   /* set of ready sockets */
	struct timeval		tval;		   /* timeout value */


	/* get port of server */
	if (argc < 3) {
		printf("Using default port.\n");
		tport_h = DEFAULT_PORT;
	} else if (sscanf(argv[2], "%" SCNu16, &tport_h)!=1) {
		printf("Invalid port number. Using default.\n"); 
		tport_h = DEFAULT_PORT;
	}
	tport_n = htons(tport_h);


	/* get IP of server */
	if (!inet_aton(DEFAULT_ADDR, &sinaddr_def))
		err_fatal("Internal error while converting default IP address");
	if (argc < 2) {
		printf("Using default IP.\n");
		sinaddr = sinaddr_def;
	} else if (!inet_aton(argv[1], &sinaddr)) {
		printf("Invalid IP address. Using default\n");
		sinaddr = sinaddr_def;
	}

	/* initialize client UDP socket */
	s = udp_client_init(sinaddr, tport_n);

	/* main client loop */	
	for (;;)
	{
	    int			len, n;
	    unsigned int 	input, output;
	    xdrhyper		req;		/* request data */
	    Response		res;		/* response data */
	    XDR 		xdrs_in;	/* input XDR stream */
	    XDR			xdrs_out;	/* output XDR stream */


	    printf(" Enter integer: ");
	    fgets(buf, BUFLEN, stdin);
	    sscanf(buf, "%u", &input);
	    req = input;
            //to define the encoding stream,and the xdrs_out point to the buf,it will write the result to the buf after converting to the  xdr format. 
	    xdrmem_create(&xdrs_out, buf, BUFLEN, XDR_ENCODE);
             //it will convert the req to xdr format in the stream xdr_out,and the function xdr_xdrhyper is generated automaticly according to the xdr file.
	    if (!xdr_xdrhyper(&xdrs_out, &req)) {
	        xdr_destroy(&xdrs_out);
	        return -1;
	    }

	    len = xdr_getpos(&xdrs_out);
	    n=send(s, buf, len, 0);
	    if (n != len)
	    {
		printf("Write error\n");
		xdr_destroy(&xdrs_out);
		continue;
	    }
	    xdr_destroy(&xdrs_out);

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
	    	n=recv(s,rbuf,BUFLEN-1,0);
                if (n != -1)
	    	{
			printf("Received response (%d bytes)\n", n);
			xdrmem_create(&xdrs_in, rbuf, n, XDR_DECODE);
			if (!xdr_Response(&xdrs_in, &res)) {
				printf("Error in decoding response\n");
			} else {
				input = res.request;
				output = res.response;
				printf("Request: %u\n", input);
				printf("Result: %u\n", output);
			}
			xdr_destroy(&xdrs_in);			
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

SOCKET udp_client_init (struct in_addr sinaddr, uint16_t tport_n) {
	SOCKET			s;		   /* socket used for communication with the server */
	struct sockaddr_in	saddr, caddr;  	   /* server and client addresses */
   	int 			result, namelen;


	/* Initialize socket API if needed */
	SockStartup();

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
	saddr.sin_addr 	      = sinaddr;

	/* set destination address in socket */
	result = connect(s, (struct sockaddr *) &saddr, sizeof(saddr));
	if (result == -1)
		err_fatal("connect() failed");

	return s;

}

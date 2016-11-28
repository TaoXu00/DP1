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
#include    "errlib.h"
#include    "sockwrap.h"

#define BUFLEN 256
//#define TIMEOUT 15
#define TIMEOUT 3
char *prog_name;
void showAddr(char *str, struct sockaddr_in *a)
{
    char *p;
    
    p = inet_ntoa(a->sin_addr);
	printf("%s %s",str,p);
	printf(":%u\n", ntohs(a->sin_port));
}
void listen_Timeout(int s)
{
 
} 
/* FUNCTION PROTOTYPES */

int main(int argc,char **argv)
{
	char	 		rbuf[BUFLEN];	   /* reception buffer */

	uint32_t		taddr_n;  /* server IP addr. (net/host byte order resp.) */
	uint16_t		tport_n, tport_h;  /* server port number */

	int			s;
        //Modify 
	int counter=0;
     struct sockaddr_in	saddr;
        fd_set			cset;
	struct timeval		tval;
          
	/* Initialize socket API if needed */
	if(argc!=4) printf("<server IP,server port,msg>");
	/*initialize the server IP*/
        prog_name=argv[0];
        taddr_n = inet_addr(argv[1]);
        /*server port*/
        tport_h=atoi(argv[2]);
  	tport_n= htons(tport_h);

	/* create the socket */
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == -1)
		err_msg("socket() failed");
	printf("Socket created\n");

         /* prepare server address structure */
    	saddr.sin_family      = AF_INET;
	saddr.sin_port        = tport_n;
	saddr.sin_addr.s_addr = taddr_n;
        
	    int			len, n;
	    struct sockaddr_in 	from;
	    socklen_t 		addrlen;
            addrlen = sizeof(struct sockaddr_in);
	    len = strlen(argv[3]);
            FD_ZERO(&cset);
	    FD_SET(s, &cset);
	    tval.tv_sec = TIMEOUT;
	    tval.tv_usec = 0;
while(counter<=5){
	    Sendto(s, argv[3], len, 0, (struct sockaddr *) &saddr, addrlen);
            printf("data has been send\n");
	    
            
	    n = select(FD_SETSIZE, &cset, NULL, NULL, &tval);
	    if (n == -1)
		err_msg("select() failed\n");
	    if (n>0)
            {
		/* receive datagram */
		
	    	n=Recvfrom(s,rbuf,BUFLEN-1,0,(struct sockaddr *)&from,&addrlen);
                if (n != -1)
	    	{
			rbuf[n] = '\0';
			
			printf("---received string '%s'\n", rbuf
                 break;
                 }
		else printf("Error in receiving response\n");
	    }
	    else 

 printf("No response received after %d seconds,now re-transmit the request\n",TIMEOUT);
}
	   
	
	Close(s);
        exit(0);
}


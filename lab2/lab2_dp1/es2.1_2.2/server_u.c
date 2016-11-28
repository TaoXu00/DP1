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
#include    "sockwrap.h"
#include    "errlib.h"

#define BUFLEN		65536
#define INVALID_SOCKET	-1
#define MAX_ADDR_LEN 100
#define MAX_CLIENT 10
#define MAX_REQUESTS 3
typedef int SOCKET;

typedef struct{
char Client[MAX_CLIENT][MAX_ADDR_LEN];
int Request[MAX_CLIENT];
int current;
}Client_list;

char *prog_name;
/* FUNCTION PROTOTYPES */
void showAddr(char *str, struct sockaddr_in *a);
Client_list create_clientList();
int check_client(Client_list *c,char *addr);

int main(int argc,char **argv)
{
	char	 		buf[BUFLEN];		/* reception buffer */
	uint16_t 		lport_n, lport_h;	/* port where server listens */
	SOCKET			s;
	int			result, addrlen, n;
	struct sockaddr_in	saddr, from;
         Client_list            c;
	prog_name=argv[0];
	/* input server port number */
        if(argc!=2)
         err_quit("usage: %s <port>",prog_name);
	lport_h=atoi(argv[1]);
  	lport_n = htons(lport_h);

	/* create the socket */
	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET)
		err_quit("(%s)socket() failed",prog_name);
	err_msg("(%s)socket created",prog_name);

	/* bind the socket to all local IP addresses */
	saddr.sin_family      = AF_INET;
	saddr.sin_port        = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;
	showAddr("listening for UDP packets on ", &saddr);
	result = bind(s, (struct sockaddr *) &saddr, sizeof(saddr));
	if (result == -1)
		err_quit("(%s)bind() failed",prog_name);
	
        c=create_clientList();
        int i;
       /* for(i=0;i<MAX_CLIENT;i++){
                printf("init Client[%d]:%s \n",i,c.Client[i]); 
              }*/ 
	/* main server loop */
	for (;;)
	{
            err_msg("(%s)waiting for a packet...",prog_name);
	    addrlen = sizeof(struct sockaddr_in);
	    n=recvfrom(s, buf, BUFLEN-1, 0, (struct sockaddr *)&from, &addrlen);   
	    if (n != -1)
	    {  
             buf[n] = '\0';
             err_msg("(%s)---received from %s",prog_name,inet_ntoa(from.sin_addr));
	     err_msg("(%s)---received string '%s' (payloads larger than 255 bytes are truncated)",prog_name,buf);  
            /*-------check client--------------------*/
            char *addr=inet_ntoa(from.sin_addr);
            result=check_client(&c,addr);
       //[debug]      printf("times %d\n",result);
            int i;
            if(result>MAX_REQUESTS){
             err_msg("(%s)banned",prog_name);
             continue;
             }
	    	
	     if(sendto(s, buf, n, 0, (struct sockaddr *)&from, addrlen) != n)
	              err_quit("(%s)Write error while replying",prog_name);
	        else
	              err_msg("(%s)Reply sent",prog_name);
	    }
	}
         close(s);
         return 0;
}

void showAddr(char *str, struct sockaddr_in *a)
{
    char *p;
    
    p = inet_ntoa(a->sin_addr);
	err_msg("(%s)%s %s!%u",prog_name,str,p,ntohs(a->sin_port));
}
Client_list create_clientList(){
Client_list c;
int i;
for(i=0;i<MAX_CLIENT;i++)
{
 memset(c.Client[i],MAX_ADDR_LEN,0);
 c.Request[i]=0;
}
 c.current=0;
 printf("creat success\n");
 return c;
}
int check_client(Client_list *c,char *addr)
{
 int found=-1;
 int times=0;
 int i;
 for(i=0;i<MAX_CLIENT;i++){
    // printf("check Client[%d]:%s \n",i,client_list.Client[i]);  
    if(strncmp(c->Client[i],addr,MAX_ADDR_LEN)==0){
       found=i;
     //[debug]  printf("Client[%d]:%s %s found\n",i,c->Client[i],addr);
       c->Request[i]++;
       times=c->Request[i];
       break;
     } 
  }
  if(found<0){
     strncpy(c->Client[c->current],addr,MAX_ADDR_LEN);
     //[debug] printf("not found add client[%d]:%s\n",c->current,c->Client[c->current]);
     c->Request[c->current]=1;
     times=1;
     c->current=(c->current+1)%MAX_CLIENT;
   //[debug]  printf("new current %d\n",c->current);
    }
   return times;
}


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
#include     <sys/stat.h>
#include     "errlib.h"
#include     "sockwrap.h"
#include     <signal.h>
#include     <errno.h>
#define BUFLEN	128
#define TIMEOUT	15
#define INVALID_SOCKET	-1
typedef int SOCKET;

char *prog_name;
/* FUNCTION PROTOTYPES */
void showAddr(char *str, struct sockaddr_in *a);
int main(int argc,char **argv)
{
	char     	   buf[BUFLEN];		/* transmission buffer */
	char	 	   rbuf[BUFLEN];	/* reception buffer */

	uint32_t	   taddr_n;		/* server IP addr. (network byte order) */
	uint16_t	   tport_n, tport_h;	/* server port number (net/host byte order resp.) */

	SOCKET		   s;
	int		   result;
	struct sockaddr_in saddr;		/* server address structure */
	struct in_addr	   sIPaddr; 		/* server IP addr. structure */
        fd_set             cset;
        struct timeval	   tval;
         /* for errlib to know the program name */
	prog_name = argv[0];
        if(argc!=3)
              err_quit("usage %s  <dest_host> <dest_port>",prog_name);
	/* input IP address and port of server */
	result = inet_aton(argv[1], &sIPaddr);
	if (!result)
		err_ret("(%s)error-Invalid address",prog_name);

        //host port number
        tport_h=atoi(argv[2]);
  	tport_n = htons(tport_h);

	/* create the socket */
    	 
	s=Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET)
		err_ret("(%s)socket() failed",prog_name);
       /* err_msg("(%s) done.Socket fd number:%d",prog_name,s);
	printf("done. Socket fd number: %d\n",s);*/
         err_msg("(%s) socket created",prog_name);
	/* prepare address structure */
    	saddr.sin_family = AF_INET;
	saddr.sin_port   = tport_n;
	saddr.sin_addr   = sIPaddr;

        Connect(s, (struct sockaddr *) &saddr, sizeof(saddr));
        showAddr("Connected to ", &saddr);
       while(1){
       int len;
       char filename[BUFLEN];
       memset(filename,0,BUFLEN);
       memset(buf,0,BUFLEN);
        /*-------send command to the server-------*/
        printf("Insert file names (no GET), one per line (end with EOF - press CTRL+D on keyboard to enter EOF):\n") ;    
       if(fgets(filename,BUFLEN,stdin)==0){
        sprintf(buf,"%s","QUIT\r\n");
        buf[strlen(buf)]='\0';
        Writen(s,buf,strlen(buf));
        close(s);
        exit(0);
       }
       else{
        filename[strlen(filename)-1]='\0';
        strcat(filename,"\r\n");
        sprintf(buf,"GET %s",filename);
        Writen(s,buf,strlen(buf));
        err_msg("(%s)- request data has been sent",prog_name);
        }
       /*-----------recceive the data from server-----------*/
         int n;
         FD_ZERO(&cset);
	 FD_SET(s, &cset);
	 tval.tv_sec = TIMEOUT;
	 tval.tv_usec = 0;
	 n = select(FD_SETSIZE, &cset, NULL, NULL,&tval);
         if (n == -1)
		err_msg("select() failed\n");
         else if (n>0){ 
         uint32_t size,size_n,time,time_n;      
         memset(rbuf,0,BUFLEN);
         n=Readline_unbuffered(s,rbuf,BUFLEN);      
         if(n==0){
         printf("%d\n",n);
         err_msg("(%s)---connection closed by server",prog_name);
         break;
         }
         if(strcmp(rbuf,"-ERR\r\n")==0){
          err_msg("(%s)---received string '%s'",prog_name,rbuf);
          err_msg("(%s)-protocol  error:received reponse '%s'",prog_name,rbuf);
           break;
           
         }
       else{  //start to receive the data of the file
            err_msg("(%s)---received string '%s'",prog_name,rbuf);
            Readn(s,&size_n,sizeof(size_n));
            size=ntohl(size_n);
            err_msg("(%s)---received file size '%d'",prog_name,size);
            Readn(s,&time_n,sizeof(time_n));
            time=ntohl(time_n);
            err_msg("(%s)---received file timestamp '%d'",prog_name,time);

            /*-----start to recceive the content of the file-------*/
           int left=size;
           int len,nread;
           filename[strlen(filename)-2]='\0';
           FILE *fp=fopen(filename,"wb");
           if(left>BUFLEN)
             len=BUFLEN;
           else
            len=left;
            memset(rbuf,0,BUFLEN);
           while((nread=read(s,rbuf,len))>0){
              fwrite(rbuf,sizeof(char),nread,fp);
              left-=nread;
              if(left>BUFLEN)
                 len=BUFLEN;
             else
                 len=left;
             memset(rbuf,0,BUFLEN);
             }
          fclose(fp);  
          err_msg("(%s)---received file '%d' bytes,file '%s' written",prog_name,size,filename);
         /*----check the file---reopen and get the size and time-------*/
         struct stat            st;
         uint32_t               size,date;
         FILE *f=fopen(filename,"rb");
         stat(filename,&st);
        // size=htonl((uint32_t) st.st_size);
        // date=htonl((uint32_t)st.st_mtime);
         size= st.st_size;
         time=st.st_mtime;
         printf("Received file %s\n",filename);
         printf("Received file size %u\n",size);
         printf("Received file timetamp %u\n",time);
         }
     }else{
        err_msg("(%s) Timeout waiting for data from server: connection with server will be closed",prog_name);
         break;
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

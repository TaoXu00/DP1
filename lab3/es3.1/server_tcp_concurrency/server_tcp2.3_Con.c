/*
 * 	File Server0.c
 *	ECHO TCP SERVER with the following features:
 *      - Gets port from keyboard 
 *      - SEQUENTIAL: serves one client at a time
 */

#include     <stdio.h>
#include     <stdlib.h>
#include     <string.h>
#include     <inttypes.h>
#include     <rpc/xdr.h>
#include      <sys/stat.h>
#include      <sys/types.h>
#include       <sys/wait.h>
#include     "errlib.h"
#include     "sockwrap.h"
#include     <signal.h>
#define BUFLEN	1024
#define TIMEOUT 60
#define INVALID_SOCKET	-1
#define  MAX_CHILDREN 3
char *prog_name;
int   n_children;
void showAddr(char *str, struct sockaddr_in *a);
int service(int s);
/* FUNCTION PROTOTYPES */
static void sig_chld(int signo){
 int status;
 int pid;
 while((pid=waitpid(-1,&status,WNOHANG))>0){
 n_children--;
 }
return ;
}
int main(int argc,char **argv)
{   
          struct sockaddr_in 	saddr, caddr;		/* server and client address structures */ 
	  int	 		conn_request_skt;	/* socket where connections are accepted */
        
	  uint16_t 		lport_n, lport_h;	/* port where the server listens (net/host byte ord resp.) */
	  int	 		bklog = 2;		/* listen backlog */
	  int	 		s;			
	  int	 		addrlen;
          int                  childpid;
          struct  sigaction action;
          int sigact_res;
   /* Initialize socket API if needed */
	/* input server port number */
          prog_name=argv[0];
         if(argc!=2){
          err_quit("Usage:%s <port>\n",prog_name);
         }
	lport_h=atoi(argv[1]);
        lport_n = htons(lport_h);

	/* create the socket */
	s = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET)
		err_ret("(%s)socket() failed",prog_name);
	err_msg("(%s) socket created",prog_name);

	/* bind the socket to any local IP address */
	saddr.sin_family      = AF_INET;
	saddr.sin_port        = lport_n;
	saddr.sin_addr.s_addr = INADDR_ANY;
   
	Bind(s, (struct sockaddr *) &saddr, sizeof(saddr));
	showAddr("listening on ", &saddr);
	/* listen */
	//printf ("Listening at socket %d with backlog = %d \n",s,bklog);
	Listen(s, bklog);
	conn_request_skt = s;
/*-----------for the signal -----------------*/
  memset(&action,0,sizeof(action));
  action.sa_handler=sig_chld;
  sigact_res=sigaction(SIGCHLD,&action,NULL);
  if(sigact_res==-1){
  err_quit("(%s)sigaction() failed",prog_name);
 }
  n_children=0;
	/* main server loop */
	for (;;)   //for accept the new connection
	{      
    //to avoid the wait() being blocked incorrectly because the children has already been waited in the signal handler,the SIGCHLD signal is masked before entering if, and re-enabled after the if;
      sigset_t x;
      sigemptyset(&x);
      sigaddset(&x,SIGCHLD);
      sigprocmask(SIG_BLOCK,&x,NULL);
      
      if(n_children==MAX_CHILDREN){
      err_msg("(%s)-calling block",prog_name);
      int sec=10;
      while(sec!=0){ //if!=0 sleep syscall has been interrupted by a signal
      sec=sleep(sec);
      }
      int status;
      int wpid=waitpid(-1,&status,0);
      n_children--;
      }     
     //re-enable the reception of the SIGCHLD signal
       sigemptyset(&x);
       sigaddset(&x,SIGCHLD);
       sigprocmask(SIG_UNBLOCK,&x,NULL);
      //so no zombies left
        err_msg("(%s - %d) waiting for connection...",prog_name, getpid());
		/* accept next connection */
	addrlen = sizeof(struct sockaddr_in);
	s = Accept(conn_request_skt, (struct sockaddr *) &caddr, &addrlen);
		
	showAddr("- new connection from client ", &caddr);
        if((childpid=fork())<0){
             err_msg("(%s)fork() failed");
             close(s);
             continue;
        }else if(childpid>0){ //parent process
         n_children++; 
         close(s);    //only listen on the passive socket 
        }else{   //child process
        Close(conn_request_skt);
        int ret=service(s);      /*serve client */
         if(ret==0){
         err_msg("(%s) -connection terminated by client",prog_name); 
        }else{
        err_msg("(%s) -connection terminated by server",prog_name);
       }
       close(s);
       exit(0);   
       }    
    }
    return 0;
}
void showAddr(char *str, struct sockaddr_in *a)
{
    char *p;
    
    p = inet_ntoa(a->sin_addr);
	err_msg("(%s)%s %s!%u",prog_name,str,p,ntohs(a->sin_port));
}
int service(int s){
        // [file] here if define the unsigned int,for different os the length maybe different, so we need define the exact size of the type unit32_t
         char	 		             buf[BUFLEN+1];		/* reception buffer */
         int                    nread,n;
         uint32_t               size,date;
         char                   filename[BUFLEN];
         struct stat            st;
         fd_set                 cset;
         struct timeval	tval;
         int                    ret;
     /* serve the client on socket s */
       while(1){   //for one connection request for many files
       err_msg("(%s)--waiting for command...",prog_name);
       FD_ZERO(&cset);
       FD_SET(s, &cset);
       tval.tv_sec = TIMEOUT;
       tval.tv_usec = 0;
       n = select(FD_SETSIZE, &cset, NULL, NULL,&tval);
       if (n == -1){ 
       err_msg("select() failed\n");
       ret=-1;
      }
     if (n>0){
    memset(buf,0,BUFLEN);
    nread = Readline_unbuffered (s, buf, BUFLEN);
		if (nread == 0) {
			err_msg("(%s)- connection closed by client: ending service of client",prog_name);
      return 0;
		} else if (nread < 0) {
			err_ret ("(%s) error - readline() failed", prog_name);
			/* return to the caller to wait for a new connection */
                        ret=-1;
                        break;
		}else{
         
                buf[nread]='\0';
                err_msg("(%s)---received string '%s'",prog_name,buf);
                char command[BUFLEN];
                sscanf(buf,"%s",command);
     /* --------check if the command is illegal or not-------*/
               if((strcmp(command,"GET"))&&(strcmp(command,"QUIT")))
                {
                 memset(buf,0,BUFLEN);
                 sprintf(buf,"-ERR\r\n");
                 Writen(s,buf,strlen(buf));
                 err_msg("(%s)--protocol  error:received reponse '%s'\n",prog_name,command);
                 ret=1;
                 break;
                }
                
     /*---------------check the request is QUIT or the file---------------------------------------*/
             if(strcmp(buf,"QUIT\r\n")==0){
                  // err_msg("(%s)---received string '%s'",prog_name,buf);
                   err_msg("(%s)client request to terminate the connection",prog_name);
                   ret=1;
                  break;
              }
              else{
                sscanf(buf, "%*s%s",filename);
                //check if the file is exits or not.
                err_msg("(%s)---client asked to send file %s\n",prog_name,filename);
                FILE *f=fopen(filename,"rb");
                if(f==NULL)
               {
                 memset(buf,0,BUFLEN);
                 sprintf(buf,"-ERR\r\n");
                 writen(s,buf,strlen(buf));
                 err_msg("(%s)--- cannot stat() file: No such file or directory %s\n",prog_name,filename);
                 err_msg("(%s)--- sending ERR message ",prog_name);
                 ret=1;
                // close(s);
                 break;
               }
               else{
                memset(buf,0,BUFLEN);
                sprintf(buf,"+OK\r\n");
                Writen (s,buf,strlen(buf));
                err_msg("(%s)send '%s'",prog_name,buf);
               
                //get the size of the file
                stat(filename,&st);
                size=htonl((uint32_t) st.st_size);
                date=htonl((uint32_t)st.st_mtime);   
                
       
  /*--send back to the client,first compute the length of the response buffer */
           
                Writen (s,&size,sizeof(size));
                err_msg("(%s)--- sent file size '%u' -convert in network byte order -to client\n",prog_name,(uint32_t)st.st_size);
           
               Writen (s,&date,sizeof(date)); 
               err_msg("(%s)--- sent file timestamp '%u' -convert in network byte order -to client",prog_name,(uint32_t)st.st_mtime);
             memset(buf,0,BUFLEN);
             while((n = fread(buf,sizeof(char),BUFLEN,f))>0){
                writen (s,buf,n);
                memset(buf,0,BUFLEN); 
               }       
              fclose(f);
               err_msg("(%s) --- send file %s to client\n",prog_name,filename);
                }
               } 
            }
       }
       else{
          err_msg("(%s) Timeout waiting for data from client: connection with client will be closed",prog_name);
          ret=-1;
          break;
        }  
      }
 return ret;
}

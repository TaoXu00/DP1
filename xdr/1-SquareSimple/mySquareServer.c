/*xdr udp server receive the hyper integer and then compute the square and then send back*/
#include<stdio.h>
#include<sthlib.h>
#include<string.h>
#include<rpc/xdr.h>
#include<type.h>
#include<mysocket.h>
#define buflen 65536
#define default_port 2050
SOCKET Socket_init(uint16_t port);
int service (char * buf, int n, struct sockaddr_in *from, int s);
/*here buf stores the num needed computed,and from the address  to which need send back,and the s is the server socket*/
int main(int argc,char **argv){
          char   buf[buflen];
          uint16_t  lport_h,lport_n;
          SOCKET s;
          struct sockadd_in  from;
          socklen_t addrlen;
          /*get server port number*/
          lport_h=default_port;
          if(argc>2)
            if(sscanf(argv[1],"%"SCNu16,&lport_h)!=1)
             {
           printf("invaild number,use the default port.\n");
           lport_h=default_port;
          }
          lport_n=htons(lport_h);
          /*initialize the UDP server socket*/
          s=Socket_init(lport_n);
          /*main server loop */
            for(;;)
            {
              n=recvfrom(s,buf,buflen,0,(struct sockaddr  *)&from,&addrlen);
              if(n!=-1)
                if(service(buf,n,&from,s)==-1)
                  printf("service perform failed\n")
                printf("service successful\n");
            }
          }

SOCKET Socket_init(uint16_t port)
          {
            SOCKET s;
            struct sockadd_in saddr;
            int result;
            /*initialize the socket API */
            SockStartup();
            /*create a socket */
             s=socket(AF_INET,SOCK_DGRAM,IPPORTO_UDP);
             if(s==INVALID_SOCKET)
                err_fatal("socket() fail\n");
             printf("done\n");
             /*prapare the data for the socket*/
             saddr.sin_family=AF_INET;
             saddr.sin_port=port;
             saddr.sin_addr.s_addr=INADDR_ANY;
             showAddr("Binding to address:"&saddr);
             result=bind(s,struct (sockaddr *)&saddr,sizeof(saddr));
             if(result==-1)
                err_fatal("bind address failed\n");
              printf("done");
              return s;  
          }
int service (char * buf, int n, struct sockaddr_in *from, int s){
        XDR xdr_in,xdr_out; //xdr  stream
        xdrhyper req;
        Response res;
        int length;
        /* initialize XDR input stream to point to buf */
        xdr_create(&xdr_in,buf,buflen,XDR_DECODE);
        /* decode request */
        if(!xdr_xdrhyper(&xdr_in,&req))
           {
           xdr_destory(xdr_in);
           return -1;
           }
        //compute the square
        res.request=req;
        res.response=req*req;
        //send back,initialize the send back xdr_stream
       xdrmem_create(&xdr_out,buf,buflen,XDR_ENCODE);
        //convert to the stream
       if(! xdr_Response(&xdr_out,&res))
          {
          printf("Encoding error\n");
          xdr_destory(xdr_in);
          xdr_destory(xdr_out);
          return -1;
          }
       //send back to the client,first compute the length of the response buffer
         length=xdr_getpos(&xdr_out);
         if(sendto(s,buf,length,0,(struct sockaddr*)&from,sizeof(*from))!=len)
           {
           printf("writing error when replaying.\n");
           xdr_destory(&xdr_in);
           xdr_destory(&xdr_out);
           return -1;
           }
          else
           {
            printf("reply send\n");
            xdr_destory(&xdr_in);
            xdr_destory(&xdr_out);
           }   
        
        
}  

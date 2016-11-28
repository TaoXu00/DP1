# Makefile for Square client and server
#
EXE 	=	SquareClient SquareServer 
SRC 	=	SquareClient.c SquareServer.c 

all:		$(SRC) mybsdfun.o mysocket.h types.h types.c
		for f in $(EXE); do (cc -o $$f $$f.c mybsdfun.o types.c); done

mybsdfun.o:	mybsdfun.c mysocket.h
		cc -c -o mybsdfun.o mybsdfun.c

types.h:	types.xdr 
		rpcgen -h -o types.h types.xdr

types.c:	types.xdr 
		rpcgen -c -o types.c types.xdr

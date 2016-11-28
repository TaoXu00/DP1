# Makefile for echo client and server
#
EXE 	=	Client0 Server0 Server1 Server2 \
		Server3 Client1u Server0u
SRC 	=	Client0.c Server0.c Server1.c Server2.c \
		Server3.c Client1u.c Server0u.c

all:		$(SRC) mybsdfun.o mysocket.h
		for f in $(EXE); do (cc -o $$f $$f.c mybsdfun.o); done

mybsdfun.o:	mybsdfun.c mysocket.h
		cc -c -o mybsdfun.o mybsdfun.c

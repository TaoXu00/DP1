Simple application using XDR data types (computing the square of a sequence of 8-bytes integers).
The application uses the UDP protocol.
The implementation is based on the functions presented in the course
(for enhancing cross-platform portability).
This version allocates and frees memory (no memory leakage problem).

types.xdr	XDR type definitions
SquareServer.c	server side
SquareClient.c	client side
mysocket.h	header for the common functions
mybsdfun.c	function implementation for Unix-like systems
makefile	makefile
readme.txt	this file

The following steps are necessary for testing the application:
1. run the make command for compiling:
$ make
2. run the server in one window:
$ ./SquareServer
3. and the client in another wondow:
$ ./SquareClient


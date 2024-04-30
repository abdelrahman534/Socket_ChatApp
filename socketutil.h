#ifndef SOCKETUTIL_H
#define SOCKETUTIL_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include"arpa/inet.h"
#include"socketutil.c"



// Function to create a TCP IPv4 socket
int CreateTcpIpv4Socket();

// Function to create an IPv4 address structure
struct sockaddr_in* CreateIPv4Address(char* address, int port);


#endif




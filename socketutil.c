#include"socketutil.h"

int CreateTcpIpv4Socket(){
  return socket(AF_INET,SOCK_STREAM,0);
}

struct sockaddr_in* CreateIPv4Address(char* ip , int port){

  struct sockaddr_in* address = malloc(sizeof(struct sockaddr_in));
  address->sin_family = AF_INET;
  address->sin_port = htons(port);

  if(strlen(ip) == 0){
    address->sin_addr.s_addr = INADDR_ANY;
  }

    else{
          inet_pton(AF_INET, ip , (struct sockaddr *) &address->sin_addr.s_addr); 
        // please convert the presentation form to 
        //unsigned integer and put it in the address that we are giving a pointer for

    }

  return address;
}


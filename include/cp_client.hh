#ifndef CP_CLIENT_HH
#define CP_CLIENT_HH

#include <iostream>
#include "cross_platform_sockets.hh"

class Client {
public:
  Client(char* host, char* port);

private:
  int Init(char* host, char* port);
};


#endif
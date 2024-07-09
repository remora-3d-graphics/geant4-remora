#ifndef CP_SERVER_HH
#define CP_SERVER_HH

#include <iostream>
#include "cross_platform_sockets.hh"

#include <list>
#include <thread>

class Server {
public:
  Server();
  ~Server();

private:
  int Init();
  void AcceptConnections();
  void SendMessages();

  void Stop(){ running = false; }

  int listenSocket;

  std::list<int> newSockets;
  std::list<int> sockets;

  std::thread listenThread;
  std::thread sendDataThread;

  bool running = true;
};

#endif // ! CP_SERVER_HH
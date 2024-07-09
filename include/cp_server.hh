#ifndef CP_SERVER_HH
#define CP_SERVER_HH

#include <iostream>
#include "cross_platform_sockets.hh"

#include <list>
#include <thread>
#include <string>

class Server {
public:
  Server();
  ~Server();

private:
  int Init();
  void AcceptConnections();
  void SendMessages();

  int SendWelcomeMessage(int newSocket);
  int SendToAll(std::string msg){};

  void Stop(){ running = false; }

  int listenSocket;

  std::list<int> newSockets;
  std::list<int> sockets;

  std::thread listenThread;
  std::thread sendDataThread;

  bool running = true;
};

#endif // ! CP_SERVER_HH
#ifndef CP_SERVER_HH
#define CP_SERVER_HH

#include <iostream>
#include "cross_platform_sockets.hh"

#include <list>
#include <thread>
#include <string>
#include <sstream>
#include <chrono>
#include <queue>

class Server {
public:
  Server();
  ~Server();

  int QueueMessageToBeSent(std::string msg){ messagesToBeSent.push(msg); };

private:
  int Init();
  void AcceptConnections();
  void SendMessages();

  int SendWelcomeMessage(int newSocket);
  int SendToAll(std::string msg);

  void Stop(){ running = false; }

  int listenSocket;

  std::list<int> newSockets;
  std::list<int> sockets;

  std::thread listenThread;
  std::thread sendDataThread;

  std::queue<std::string> messagesToBeSent;

  bool running = true;
};

#endif // ! CP_SERVER_HH
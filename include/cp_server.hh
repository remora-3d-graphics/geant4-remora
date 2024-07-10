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

// Geant4 includes
#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"

namespace remora {

  class RemoraMessenger;

	class Server {
	public:
		Server();
		~Server();

		void QueueMessageToBeSent(std::string msg) { messagesToBeSent.push(msg); };

	private:
		int Init();
		void AcceptConnections();
		void SendMessages();

		void Stop() { running = false; }

		int SendWelcomeMessage(int newSocket);
		int SendToAll(std::string msg);

    int SendDetectors(int sock);
    int SendTracks(){ return 0; };

		int listenSocket;

		std::list<int> newSockets;
		std::list<int> sockets;

		std::thread listenThread;
		std::thread sendDataThread;

		std::queue<std::string> messagesToBeSent;

		bool running = true;

		RemoraMessenger* remoraMessenger;
	};
};

#endif // ! CP_SERVER_HH
#ifndef CP_SERVER_HH
#define CP_SERVER_HH

#include "cross_platform_sockets.hh"
#include "json.hpp"

#include <iostream>
#include <list>
#include <thread>
#include <string>
#include <sstream>
#include <chrono>
#include <queue>
#include <mutex>

// Geant4 includes
#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4Point3D.hh"
#include "G4VSolid.hh"
#include "G4Polyhedron.hh"

using json = nlohmann::json;

namespace remora {

  class RemoraMessenger;

	class Server {
	public:
		Server();
		~Server();

		void QueueMessageToBeSent(std::string msg);

	private:
		int Init();
		void AcceptConnections();
		void SendMessages();

		int listenSocket;

		void Stop() { running = false; }

		int SendWelcomeMessage(int newSocket);
		int SendToAll(std::string msg);

    int SendDetectors(int sock=-1);
    int SendOneDetector(G4VPhysicalVolume* volume, int sock=-1);
    json GetJsonFromSolid(const G4VSolid* solid);

    int SendTracks(){ return 0; };

    // client thread stuff
    void AllocateThreadsLoop();
    int nThreads = 0;
    int nClientsReceived = 0;
    void ClientLoop(int sock);
    void ManageMessagesLoop();

    std::mutex messageQueueMutex;
    std::mutex nThreadsMutex;
    std::mutex nClientsReceivedMutex;

    // functions that need a mutex
    int ViewNMessages();
    std::string ViewNextMessage();
    void PopNextMessage();

    int ViewNThreads();
    int ViewNClientsReceived();
    void AddToNThreads(int num);
    void AddToNClientsReceived(int num);
    void SetNClientsReceived(int num);

		std::list<int> newSockets;
		std::list<int> sockets;

		std::thread listenThread;
		std::thread sendDataThread;
    std::thread allocatorThread;
    std::thread manageMessagesThread;

		std::queue<std::string> messagesToBeSent;

		bool running = true;
    bool g4runInitialized = false;

		RemoraMessenger* remoraMessenger;

    friend class RemoraMessenger;
	};
};

#endif // ! CP_SERVER_HH
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
#include <atomic>

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
    json GetJsonFromVolume(const G4VPhysicalVolume* volume);

    int SendTracks(){ return 0; };

    // client thread stuff
    void AllocateThreadsLoop();
    std::atomic<int> nThreads = 0;
    std::atomic<int> nClientsReceived = 0;
    void ClientLoop(int sock);
    void ManageMessagesLoop();

    std::mutex newClientsMutex;
    std::mutex messageQueueMutex;

    // functions that need a mutex
    int ViewNMessages();
    std::string ViewNextMessage();
    void PopNextMessage();

    int ViewNNewClients();
    void PushNewClient(int sock);
    int PopNewClient();

		std::list<int> newSockets; // todo: QUEUE
		std::list<int> sockets;

		std::thread listenThread;
		std::thread sendDataThread;
    std::thread allocatorThread;
    std::thread manageMessagesThread;
    std::thread timeOutThread;

		std::queue<std::string> messagesToBeSent;

		std::atomic<bool> running = true;
    bool g4runInitialized = false;

		RemoraMessenger* remoraMessenger;

    friend class RemoraMessenger;
	};
};

#endif // ! CP_SERVER_HH
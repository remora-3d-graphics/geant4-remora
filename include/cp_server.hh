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
#include <shared_mutex>
#include <map>

// Geant4 includes
#include "G4RunManager.hh"
#include "G4RunManagerFactory.hh"
#include "G4Point3D.hh"
#include "G4VSolid.hh"
#include "G4Polyhedron.hh"

// remora includes
#include "RemoraSteppingAction.hh"
#include "RemoraTrajectory.hh"


using json = nlohmann::json;

namespace remora {

  class RemoraMessenger;

	class Server {
	public:
		Server();
		~Server();

		void QueueMessageToBeSent(std::string msg);

    RemoraSteppingAction* GetRemoraSteppingAction(G4UserSteppingAction* prevSteppingAction=nullptr);

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

    // send trajectory stuff
    std::map<int, Trajectory> trajsBeingBuilt;
    std::queue<Trajectory> finishedTrajs;

    // client thread stuff
    void AllocateThreadsLoop();
    std::atomic<int> nThreads = 0;
    std::atomic<int> nClientsReceived = 0;
    void ClientLoop(int sock);
    void ManageMessagesLoop();

    std::mutex newClientsWriteMutex;
    std::shared_mutex newClientsReadMutex;
    std::mutex messageQueueWriteMutex;
    std::shared_mutex messageQueueReadMutex;

    // functions that need a mutex
    int ViewNMessages();
    std::string ViewNextMessage();
    void PopNextMessage();

    int ViewNNewClients();
    void PushNewClient(int sock);
    int PopNewClient();

    std::unordered_map<int, int> clientsUnsent; // tells clients how many messages they haven't sent yet
    std::shared_mutex clientsUnsentMutex;
    std::mutex masterUnsentMutex;

    void AddClientToUnsent(unsigned int clientSock);
    void RemoveClientFromUnsent(unsigned int clientSock);
    void AddMessageToUnsent(unsigned int num);
    unsigned int ClientAccessNUnsent(unsigned int clientSock);
    void ClientSubtractFromUnsent(unsigned int clientSock);

    void KillClientThread(int sock);

		std::list<int> newSockets; // todo: QUEUE
		std::list<int> sockets;

		std::thread listenThread;
		std::thread sendDataThread;
    std::thread allocatorThread;
    std::thread manageMessagesThread;
    std::thread timeOutThread; // todo... seriously though

		std::queue<std::string> messagesToBeSent;

		std::atomic<bool> running = true;
    bool g4runInitialized = false;

		RemoraMessenger* remoraMessenger;

    friend class RemoraMessenger;
	};
};

#endif // ! CP_SERVER_HH
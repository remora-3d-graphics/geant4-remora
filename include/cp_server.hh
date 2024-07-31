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
#include "MessageManager.hh"


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

		int listenSocket;

		void Stop() { running = false; }

		int SendWelcomeMessage(int newSocket);

    // send detectors stuff
    int SendDetectors(int sock=-1);
    int SendOneDetector(G4VPhysicalVolume* volume, int sock=-1);
    json GetJsonFromVolume(const G4VPhysicalVolume* volume);

    // send trajectory stuff
    TrajectoryManager trajManager;
    void SendTrajsLoop();
    bool SendOneTraj(Trajectory* traj);
    json GetTrajJson(Trajectory* traj);

    // client thread stuff
    void ClientLoop(int sock);
    void KillClientThread(int sock);

		std::list<int> newSockets; // todo: QUEUE
		std::list<int> sockets;

		std::thread listenThread;
    std::thread sendTrajsThread;

    MessageManager messageManager;

		std::atomic<bool> running = true;
    bool g4runInitialized = false;

		RemoraMessenger* remoraMessenger;

    friend class RemoraMessenger;
	};
};

#endif // ! CP_SERVER_HH
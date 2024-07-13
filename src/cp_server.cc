#include "cp_server.hh"
#include "RemoraG4Messenger.hh"

namespace remora {
  Server::Server() {
    std::cout << "Hello Server" << std::endl;

    // initialize the messenger
    remoraMessenger = new RemoraMessenger(this);

    // currently the Init() function blocks Geant4 execution. TODO: put it on a thread
    if (Init() != 0) {
      std::cout
        << "Server did not initialize properly. "
        << std::endl
        << "To use a server for visualization, please restart the app."
        << std::endl;

      return;
    }

    // runs concurrently with the rest of the main function
    listenThread = std::thread(&Server::AcceptConnections, this);
    sendDataThread = std::thread(&Server::SendMessages, this);
  }

  Server::~Server() {
    Stop();
    listenThread.join();
    sendDataThread.join();

    delete remoraMessenger;
  }

  void Server::AcceptConnections() {
    while (running) {

      int clientSocket = -1;
      clientSocket = accept(listenSocket, NULL, NULL);

      if (clientSocket == -1) {
        std::cout << "Accept failed" << std::endl;
      }

      std::cout << "client connected" << std::endl;

      newSockets.push_back(clientSocket);
    }
  }

  void Server::SendMessages() {

    while (running) {
      // for new sockets
      if (newSockets.size() != 0) {
        int clientSocket = newSockets.front();
        int sent = SendWelcomeMessage(clientSocket);
        std::cout << "Sent message with code: " << sent << std::endl;
        newSockets.pop_front();
        sockets.push_back(clientSocket);
      }

      // send detectors ONLY when run is initialized


      if (messagesToBeSent.size() > 0) {
        SendToAll(messagesToBeSent.front());
        messagesToBeSent.pop();
      }

      // cp_close(clientSocket);
    }
  }

  int Server::SendDetectors(int sock){
    // get world from runManager. Note: got this line from G4VisManager.hh
    const G4VPhysicalVolume *world = G4RunManagerFactory::GetMasterRunManagerKernel()->GetCurrentWorld();

    std::cout << "World ptr: " << world << std::endl;
    if (!world){
      std::cout << "REMORA: Can't get world! Perhaps try /run/initialize first." << std::endl;
      return 1;
    }

    // get all children of the world and put them in json
    json allShapes;

    size_t nChildren = world->GetLogicalVolume()->GetNoDaughters();
    for (int i=0; i<nChildren; i++){
      G4VPhysicalVolume* volume = world->GetLogicalVolume()->GetDaughter(i);
      G4String name = volume->GetName();
      G4String type = volume->GetLogicalVolume()->GetSolid()->GetEntityType();

      std::cout << "The type is: " << type;
    }
    
    // convert them to json and send
    G4String testShape = "{\"physShape\":\
{\"vertices\":[[100, 100, 100],[100, 200, 100],[200, 200, 100],[200,100,100],\
[100, 100, 200],[100, 200, 200],[200, 200, 200],[200,100,250]],\
\"indices\":[[0, 1],[1, 2],[2, 3],[3,0],\
[4, 5],[5, 6],[6, 7],[7,4],\
[0, 4],[1, 5],[2, 6],[3,7]\
]}}";
    G4String cmd = "AddShapes" + testShape;

    if (sock == -1){ // send to all if not specified
      QueueMessageToBeSent(cmd);
    }

    return 0;
  }

  int Server::SendToAll(std::string strmsg) {

    int returnCode = 0;

    for (int s : sockets) {
      const char* msg = strmsg.c_str();
      int len, bytes_sent;

      len = strlen(msg);
      bytes_sent = send(s, msg, len, 0);

      if (bytes_sent != len) {
        std::cout << "The whole message wasn't quite sent!" << std::endl;
        // return 1;
      }
      else {
        std::cout << "Sent message" << std::endl;
      }
    }

    return returnCode;
  }

  int Server::SendWelcomeMessage(int clientSocket) {
    // send a message over
    std::string msg = "Welcome";

    int len = msg.size();
    int bytes_sent = send(clientSocket, msg.data(), len, 0);

    if (bytes_sent != len) {
      std::cout << "The whole message wasn't quite sent!" << std::endl;
      // return 1;
    }
    else {
      std::cout << "Sent message" << std::endl;
    }

    char response[1024] = { 0 };

    int bytes_recieved = -1;

    while (bytes_recieved <= 0) {
      bytes_recieved = recv(clientSocket, response, sizeof(response), 0);
    }

    std::cout << "From socket " << clientSocket << ": " << response << std::endl;

    return 0;
  }

  int Server::Init() {
    int status;

    status = cp_init();
    if (status != 0) {
      std::cout << "Did not init correctly: " << status << std::endl;
      return 1;
    }

    struct addrinfo hints, * res, * rp;

    hints = cp_get_hints();

    status = getaddrinfo(0, "8080", &hints, &res); // hardcoded
    if (status != 0) {
      std::cout << "get addr info failed: " << status << std::endl;
      return 1;
    }

    std::cout << "get addr info succeeded" << std::endl;

    listenSocket = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
      listenSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

      if (listenSocket == -1) continue;

      if (bind(listenSocket, rp->ai_addr, rp->ai_addrlen) == 0) break;

      cp_close(listenSocket);
    }

    if (rp == NULL) {
      std::cout << "Could not bind" << std::endl;
      return 1;
    }

    std::cout << "Bound socket successfully" << std::endl;

    char hostname[100];
    char portname[100];
    getnameinfo(res->ai_addr, res->ai_addrlen, hostname, sizeof hostname, portname, sizeof portname, NI_NUMERICHOST | NI_NUMERICSERV);

    std::cout << "HOST: " << hostname << " PORT: " << portname << std::endl;

    freeaddrinfo(res);


    if (listenSocket == -1) {
      std::cout << "Error creating listen socket: " << std::endl;
      return 1;
    }

    std::cout << "listen socket created" << std::endl;

    std::cout << "Listening..." << std::endl;

    if (listen(listenSocket, 100) == -1) {
      std::cout << "Listen error" << std::endl;
      cp_close(listenSocket);
      return 1;
    }

    return 0;
  }
}
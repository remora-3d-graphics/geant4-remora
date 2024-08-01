#include "cp_server.hh"
#include "RemoraG4Messenger.hh"

namespace remora {
  Server::Server() {
    G4cout << "Hello Server" << G4endl;

    // initialize the messenger
    remoraMessenger = new RemoraMessenger(this);

    stateMachine = new RemoraStateMachine();

    // currently the Init() function blocks Geant4 execution. TODO: put it on a thread
    if (Init() != 0) {
      G4cout
        << "Server did not initialize properly. "
        << G4endl
        << "To use a server for visualization, please restart the app."
        << G4endl;

      return;
    }

    // runs concurrently with the rest of the main function
    listenThread = std::thread(&Server::AcceptConnections, this);
    sendTrajsThread = std::thread(&Server::SendTrajsLoop, this);
  }

  Server::~Server() {
    Stop();
    listenThread.join();
    sendTrajsThread.join();

    delete remoraMessenger;
  }

  RemoraSteppingAction* Server::GetRemoraSteppingAction(G4UserSteppingAction* prevSteppingAction){
    return new RemoraSteppingAction(&trajManager, prevSteppingAction);
  }

  void Server::SendTrajsLoop(){
    while (running){
      if (trajManager.GetNTrajectories() == 0) continue;

      Trajectory* nextTraj = trajManager.GetNextTrajectory();
      
      SendOneTraj(nextTraj);

      trajManager.PopNextTrajectory();
    }
  }

  bool Server::SendOneTraj(Trajectory* traj){
    json theTrajJson = GetTrajJson(traj);

    if (theTrajJson.dump() == ""){
      G4cout << "SEND TRAJ ERROR: INVALID JSON" << G4endl;
      return false;
    }

    std::string msg = "AddShapes" + theTrajJson.dump();
    QueueMessageToBeSent(msg);
    return true;
  }

  json Server::GetTrajJson(Trajectory* traj){
    /*FORMAT:
    {
    "THE_SHAPE'S_NAME": {
      "vertices": [
        [0, 0, 9],
        [1, 2, 3],
        [3, 4, 5]
      ],
      "indices": [
        [0, 1],
        [1, 2],
        [2, 1]
      ]
      }
    }
    */

    auto formatCoord = [](float coord) {
      return std::to_string(static_cast<int>(coord));
    };


    G4String theJson = "{\"traj";
    theJson += traj->name;
    theJson += std::to_string(traj->id);
    theJson += "\":{\"vertices\":[";

    // save for later before I start popping
    int nPoints = traj->points.size();

    while (!traj->points.empty()){
      G4ThreeVector nextPt = traj->points.front();
      traj->points.pop();
      theJson += "[";
      theJson += formatCoord(nextPt.getX());
      theJson += ",";
      theJson += formatCoord(nextPt.getY());
      theJson += ",";
      theJson += formatCoord(nextPt.getZ());
      theJson += "],";
    }
    theJson.pop_back();
    theJson += "],\"indices\":[";
    for (int i=0; i < nPoints-1; i++){
      theJson += "[";
      theJson += std::to_string(i);
      theJson += ",";
      theJson += std::to_string(i+1);
      theJson += "],";
    }
    theJson.pop_back();
    theJson += "]}}";

    json finalJson;
    try {
      finalJson = json::parse(theJson);
    } catch(json::parse_error){
      finalJson = "{}"_json;
    }
    return finalJson;
  }

  void Server::ClientLoop(int sock){
    int attempts = 0;

    // send welcome message
    int sent = SendWelcomeMessage(sock);
    G4cout << "Sent message with code: " << sent << G4endl;
    if (sent != 0) return;

    while (running){
      // send and then wait for response
      if (!messageManager.MessagesWaiting(sock)) continue;

      std::string msgToSend = messageManager.GetNextMessage(sock);

      // G4cout << "SENDING " << msgToSend << " from thread " << sock << G4endl;
      
      // send in chunks!
      int chunkSize = 1000;

      const char* msg = msgToSend.c_str();
      int len = strlen(msg);
      int bytes_sent = 0;
      int total_sent = 0;

      while (total_sent < len) {
        int remaining = len - total_sent;
        int to_send = remaining < chunkSize ? remaining : chunkSize;

        bytes_sent = send(sock, msg + total_sent, to_send, 0);
        if (bytes_sent == -1) {
          // Handle error
          std::cerr << "Error sending message" << G4endl;
          break;
        }
        total_sent += bytes_sent;
      }

      char buff[10] = {0};
      int bytesReceived = -1;

      bytesReceived = recv(sock, buff, sizeof(buff), 0);

      if (bytesReceived <= 0){
        KillClientThread(sock);
        break;
      }

      if (std::strcmp(buff, "REMORA(0)") == 0){
        // success!
        messageManager.PopNextMessage(sock);
        attempts = 0;
      }
      else if (attempts > 6){
        // kill thread
        KillClientThread(sock);
        break;
      }
      else if (std::strcmp(buff, "bye") == 0){
        // client wants to leave
        KillClientThread(sock);
        break;
      }
      else {
        // try again
        G4cout << "ERROR: client said: " << buff << G4endl;
        attempts++;
      }
    }
  }

  void Server::KillClientThread(int sock){
    messageManager.RemoveClient(sock);
    cp_close(sock);
    G4cout << "Client " << sock << "disconnected." << G4endl;
  }

  void Server::QueueMessageToBeSent(std::string msg){
    // enforce the wrapper
    std::ostringstream oss;
    oss << "REMORA(" << msg << ")";
    std::string formattedString = oss.str();

    if (!messageManager.QueueMessageForAll(formattedString)){
      G4cout << "Can't queue message, no clients connected!" << G4endl;
    }
  }

  void Server::AcceptConnections() {
    while (running) {

      int clientSocket = -1;
      clientSocket = accept(listenSocket, NULL, NULL);

      if (clientSocket == -1) {
        G4cout << "Accept failed" << G4endl;
      }

      G4cout << "client connected" << G4endl;

      messageManager.AddNewClient(clientSocket);

      // detach a thread for it
      std::thread(&Server::ClientLoop, this, clientSocket).detach();
    }
  }

  int Server::SendDetectors(int sock){
    // get world from runManager. Note: got this line from G4VisManager.hh
    const G4VPhysicalVolume *world = G4RunManagerFactory::GetMasterRunManagerKernel()->GetCurrentWorld();

    if (!world){
      G4cout << "REMORA: Can't get world! Perhaps try /run/initialize first." << G4endl;
      return 1;
    }

    size_t nChildren = world->GetLogicalVolume()->GetNoDaughters();
    for (int i=0; i<nChildren; i++){
      G4VPhysicalVolume* volume = world->GetLogicalVolume()->GetDaughter(i);
      SendOneDetector(volume, sock);
    }

    return 0;
  }

  int Server::SendOneDetector(G4VPhysicalVolume* volume, int sock){
    if (sock == -1){ // send to all
      json wrapper;

      G4String name = volume->GetName();

      json shapeJson = GetJsonFromVolume(volume);
      wrapper[name] = shapeJson;

      G4String cmd = "AddShapes" + wrapper.dump();
      QueueMessageToBeSent(cmd);
    }
    return 0;
  }

  json Server::GetJsonFromVolume(const G4VPhysicalVolume* volume){
    json solidJson;

    G4VSolid* solid = volume->GetLogicalVolume()->GetSolid();

    G4Polyhedron* polyhedron = solid->CreatePolyhedron();

    /*
    FROM G4 DOCUMENTATION
    GetObjectRotationValue() const;  //  Replacement
    G4ThreeVector  GetObjectTranslation() const;
    */
    G4ThreeVector pos = volume->GetObjectTranslation();
    G4RotationMatrix rot = volume->GetObjectRotationValue();

    // lambda function to convert double into int then string
    auto format = [](G4double d){
      return std::to_string(static_cast<int>(d));
    };

    auto rotateAndMove = [](G4ThreeVector vertex_, G4ThreeVector pos_, G4RotationMatrix rot_){
      // rotate, then move.
      return rot_(vertex_) + pos_;
    };

    // create JSON:
    std::string theJson = "{\"vertices\":[";

    // Get vertices
    G4int numVertices = polyhedron->GetNoVertices();
    // note: vertices are one indexed
    for (G4int i=1; i < numVertices+1; i++) {
      G4ThreeVector theVertex = rotateAndMove(polyhedron->GetVertex(i), pos, rot);

      theJson += "[";
      theJson += format(theVertex.x());
      theJson += ",";
      theJson += format(theVertex.y());
      theJson += ",";
      theJson += format(theVertex.z());
      if (i == numVertices){ 
        // the last one
        theJson += "]";
      } else {
        theJson += "],";
      }
    }
    theJson += "],";

    theJson += "\"indices\":[";

    std::vector<std::pair<G4int, G4int>> edgeIndices;
    G4int flags = 0;
    G4int edgei[2];
    while (polyhedron->GetNextEdgeIndices(edgei[0], edgei[1], flags)){
      theJson += "[";
      // note the -1 because these are 1 indexed
      theJson += std::to_string(edgei[0]-1);
      theJson += ",";
      theJson += std::to_string(edgei[1]-1);
      theJson += "],";
    }
    // get rid of that last comma
    theJson.pop_back();

    theJson += "]}";

    solidJson = json::parse(theJson);
    return solidJson;
  }


  int Server::SendWelcomeMessage(int clientSocket) {
    // send a message over
    std::string msg = "REMORA(Welcome)";

    int len = msg.size();
    int bytes_sent = send(clientSocket, msg.data(), len, 0);

    if (bytes_sent != len) {
      G4cout << "The whole message wasn't quite sent!" << G4endl;
      // return 1;
    }
    else {
      G4cout << "Sent message" << G4endl;
    }

    char response[1024] = { 0 };

    int bytesReceived = -1;

    bytesReceived = recv(clientSocket, response, sizeof(response), 0);

    if (bytesReceived <= 0){
      G4cout << "Socket closed by client" << G4endl;
      return 1;
    }

    G4cout << "From socket " << clientSocket << ": " << response << G4endl;

    return 0;
  }

  int Server::Init() {
    int status;

    status = cp_init();
    if (status != 0) {
      G4cout << "Did not init correctly: " << status << G4endl;
      return 1;
    }

    struct addrinfo hints, * res, * rp;

    hints = cp_get_hints();

    status = getaddrinfo(0, "8080", &hints, &res); // hardcoded
    if (status != 0) {
      G4cout << "get addr info failed: " << status << G4endl;
      return 1;
    }

    G4cout << "get addr info succeeded" << G4endl;

    listenSocket = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
      listenSocket = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);

      if (listenSocket == -1) continue;

      if (bind(listenSocket, rp->ai_addr, rp->ai_addrlen) == 0) break;

      cp_close(listenSocket);
    }

    if (rp == NULL) {
      G4cout << "Could not bind" << G4endl;
      return 1;
    }

    G4cout << "Bound socket successfully" << G4endl;

    char hostname[100];
    char portname[100];
    getnameinfo(res->ai_addr, res->ai_addrlen, hostname, sizeof hostname, portname, sizeof portname, NI_NUMERICHOST | NI_NUMERICSERV);

    G4cout << "HOST: " << hostname << " PORT: " << portname << G4endl;

    freeaddrinfo(res);


    if (listenSocket == -1) {
      G4cout << "Error creating listen socket: " << G4endl;
      return 1;
    }

    G4cout << "listen socket created" << G4endl;

    G4cout << "Listening..." << G4endl;

    if (listen(listenSocket, 100) == -1) {
      G4cout << "Listen error" << G4endl;
      cp_close(listenSocket);
      return 1;
    }

    return 0;
  }

} // ! namespace remora
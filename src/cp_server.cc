#include "cp_server.hh"
#include "RemoraG4Messenger.hh"

namespace remora {
  Server::Server() {
    std::cout << "Hello Server" << std::endl;

    // initialize the messenger
    remoraMessenger = new RemoraMessenger(this);

    if (G4Init() != 0){
      std::cout
      << "Geant4 parts didn't init properly." << std::endl;
    }

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
    allocatorThread = std::thread(&Server::AllocateThreadsLoop, this);
    manageMessagesThread = std::thread(&Server::ManageMessagesLoop, this);
  }

  Server::~Server() {
    Stop();
    listenThread.join();
    allocatorThread.join();
    manageMessagesThread.join();
    // sendDataThread.join();

    delete remoraMessenger;
  }

  void Server::AllocateThreadsLoop(){
    while (running){
      if (ViewNNewClients() == 0) continue;

      // allocate thread for new sockets

      // only allocate if there are no messages in queue
      while (ViewNMessages() != 0){}

      int newClient = PopNewClient();

      // create an unsent entry for them
      AddClientToUnsent(newClient);

      std::thread(&Server::ClientLoop, this, newClient).detach();

      nThreads++;
    }
  }

  void Server::ClientLoop(int sock){
    int attempts = 0;

    // send welcome message
    int sent = SendWelcomeMessage(sock);
    std::cout << "Sent message with code: " << sent << std::endl;
    if (sent != 0) return;

    while (running){
      // send and then wait for response
      if (ViewNMessages() == 0) continue;

      // if we have none to send, continue
      if (ClientAccessNUnsent(sock) == 0) continue;

      std::string msgToSend = ViewNextMessage();

      std::cout << "SENDING " << msgToSend << " from thread " << sock << std::endl;
      
      // TODO::::: send in chunks!
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
          std::cerr << "Error sending message" << std::endl;
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
        std::cout << "Success!" << std::endl;
        ClientSubtractFromUnsent(sock);
        nClientsReceived++;
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
        std::cout << "ERROR: client said: " << buff << std::endl;
        attempts++;
      }
    }
  }

  void Server::KillClientThread(int sock){
    nThreads--;
    cp_close(sock);
    RemoveClientFromUnsent(sock);
    std::cout << "Client " << sock << "disconnected." << std::endl;
  }

  void Server::ManageMessagesLoop(){
    while (running){
      if (ViewNMessages() == 0) continue;

      // clear out messages if there are no clients connected
      if (nThreads == 0){
        PopNextMessage();
        continue;
      }

      if (nThreads == nClientsReceived) {
        // msg sent to all threads
        nClientsReceived = 0;
        PopNextMessage();
      }
    }
  }
  

  void Server::QueueMessageToBeSent(std::string msg){
    // enforce the wrapper
    std::ostringstream oss;
    oss << "REMORA(" << msg << ")";
    std::string formattedString = oss.str();

    // make sure there are some clients
    if (nThreads == 0){
      std::cout << "Could not add message to queue, there are no clients connected." << std::endl;
      return;
    }

    // mutex lock it
    std::unique_lock<std::mutex> lock(messageQueueWriteMutex);
    messagesToBeSent.push(formattedString);

    // add to unsent so the clients know there's one waiting
    AddMessageToUnsent(1);
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

      // for now just a debug print
      std::cout 
      << "DEBUG: "
      << "N Threads: "
      << nThreads
      << " N Received: "
      << nClientsReceived
      << " front of queue: "
      << ViewNextMessage()
      << std::endl;

      std::this_thread::sleep_for(std::chrono::seconds(3));


      continue;
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

      // debug print
      std::cout << wrapper << std::endl;

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
    std::string msg = "REMORA(Welcome)";

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

    int bytesReceived = -1;

    bytesReceived = recv(clientSocket, response, sizeof(response), 0);

    if (bytesReceived <= 0){
      std::cout << "Socket closed by client" << std::endl;
      return 1;
    }

    std::cout << "From socket " << clientSocket << ": " << response << std::endl;

    return 0;
  }

  int Server::G4Init(){
    // one option is to get the user action init and then define a derived class and then
    // set the action initialization back
    // then this function really should block execution.
    G4VUserActionInitialization *actionInit = G4RunManagerFactory::GetMasterRunManager()->GetNonConstUserActionInitialization();

    // G4RunManagerFactory::GetMasterRunManager()->SetUserInitialization(new RemoraActionInit(actionInit));
    // G4RunManager::GetRunManager()->SetUserInitialization(new RemoraActionInit(actionInit));

    G4RunManagerFactory::GetMTMasterRunManager()->SetUserInitialization(new RemoraActionInit(actionInit));

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


  // Mutex management
  int Server::ViewNMessages(){
    std::shared_lock<std::shared_mutex> lock(messageQueueReadMutex);

    return messagesToBeSent.size();
  }

  std::string Server::ViewNextMessage(){
    std::shared_lock<std::shared_mutex> lock(messageQueueReadMutex);

    if (messagesToBeSent.empty()){
      return "";
    }

    return messagesToBeSent.front();
  }

  void Server::PopNextMessage(){
    std::unique_lock<std::mutex> lock(messageQueueWriteMutex);

    if (messagesToBeSent.empty()){
      std::cout << "Cant pop, message queue empty!" << std::endl;
      return;
    }

    messagesToBeSent.pop();
  }

  int Server::ViewNNewClients(){
    std::shared_lock<std::shared_mutex> lock(newClientsReadMutex);

    return newSockets.size();
  }

  void Server::PushNewClient(int sock){
    std::unique_lock<std::mutex> lock(newClientsWriteMutex);

    newSockets.push_back(sock);
  }

  int Server::PopNewClient(){
    std::unique_lock<std::mutex> lock(newClientsWriteMutex);

    int newClient;
    if (newSockets.empty()){
      std::cout << "Can't pop newSockets, it's empty!" << std::endl;
      return -1;
    }

    newClient = newSockets.front();
    newSockets.pop_front();

    return newClient;
  }


  void Server::AddClientToUnsent(unsigned int clientSock){
    std::unique_lock<std::mutex> lock(masterUnsentMutex);
    clientsUnsent[clientSock] = 0;
  }

  void Server::RemoveClientFromUnsent(unsigned int clientSock){
    std::unique_lock<std::mutex> lock(masterUnsentMutex);
    if (clientsUnsent.find(clientSock) != clientsUnsent.end()){
      clientsUnsent.erase(clientSock);
    }
    else {
      std::cout << "RemoveClientFromUnsent error, client not found in map" << std::endl;
    }
  }

  void Server::AddMessageToUnsent(unsigned int num){
    std::unique_lock<std::mutex> lock(masterUnsentMutex);
    for (auto& entry : clientsUnsent){
      entry.second += 1;
    }
  }

  unsigned int Server::ClientAccessNUnsent(unsigned int clientSock){
    std::shared_lock<std::shared_mutex> lock(clientsUnsentMutex);

    return clientsUnsent[clientSock];
  }

  void Server::ClientSubtractFromUnsent(unsigned int clientSock){
    std::shared_lock<std::shared_mutex> lock(clientsUnsentMutex);

    clientsUnsent[clientSock] -= 1;
  }

} // ! namespace remora
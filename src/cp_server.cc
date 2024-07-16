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
      std::thread(&Server::ClientLoop, this, newClient).detach();

      AddToNThreads(1);
    }
  }

  void Server::ClientLoop(int sock){
    std::string lastMessageSent;
    int attempts = 0;

    // send welcome message
    int sent = SendWelcomeMessage(sock);
    std::cout << "Sent message with code: " << sent << std::endl;
    if (sent != 0) return;

    while (running){
      // send and then wait for response
      if (ViewNMessages() == 0) continue;
      if (ViewNextMessage() == lastMessageSent) continue;

      std::string msgToSend = ViewNextMessage();

      std::cout << "SENDING " << msgToSend << " from thread " << sock << std::endl;
      
      const char* msg = msgToSend.c_str();
      int len, bytes_sent;

      len = strlen(msg);
      bytes_sent = send(sock, msg, len, 0);

      if (bytes_sent != len) {
        std::cout << "The whole message wasn't quite sent!" << std::endl;
        // return 1;
      }
      else {
        std::cout << "Sent message" << std::endl;
      }

      char buff[10] = {0};
      int bytesReceived = -1;

      bytesReceived = recv(sock, buff, sizeof(buff), 0);

      if (bytesReceived <= 0){
        std::cout << "Client closed connection. Killing thread." << std::endl;
        AddToNThreads(-1);
        cp_close(sock);
        std::cout << "Thread killed." << std::endl;
        break;
      }

      if (std::strcmp(buff, "REMORA(0)") == 0){
        // success!
        std::cout << "Success!" << std::endl;
        lastMessageSent = msgToSend;
        AddToNClientsReceived(1);
        attempts = 0;
      }
      else if (attempts > 6){
        // kill thread
        std::cout << "Socket: " << sock << " disconnected after " << attempts << " tries. " << std::endl;
        AddToNThreads(-1);
        cp_close(sock);
        std::cout << "Thread killed" << std::endl;
        break;
      }
      else if (std::strcmp(buff, "bye") == 0){
        // client wants to leave
        std::cout << "Socket: " << sock << " disconnected." << std::endl;
        AddToNThreads(-1);
        cp_close(sock);
        std::cout << "Thread killed." << std::endl;
        break;
      }
      else {
        // try again
        std::cout << "ERROR: client said: " << buff << std::endl;
        attempts++;
      }
    }
  }

  void Server::ManageMessagesLoop(){
    while (running){
      if (ViewNMessages() == 0) continue;

      // clear out messages if there are no clients connected
      if (ViewNThreads() == 0){
        PopNextMessage();
        continue;
      }

      if (ViewNThreads() == ViewNClientsReceived()) {
        // msg sent to all threads
        SetNClientsReceived(0);
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
    if (ViewNThreads() == 0){
      std::cout << "Could not add message to queue, there are no clients connected." << std::endl;
      return;
    }

    // mutex lock it
    std::lock_guard<std::mutex> lock(messageQueueMutex);
    messagesToBeSent.push(formattedString);
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
      << ViewNThreads()
      << " N Received: "
      << ViewNClientsReceived()
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
      json shapeJson = GetJsonFromSolid(volume->GetLogicalVolume()->GetSolid());
      wrapper[name] = shapeJson;

      // debug print
      std::cout << wrapper << std::endl;

      G4String cmd = "AddShapes" + wrapper.dump();
      QueueMessageToBeSent(cmd);
    }
    return 0;
  }

  json Server::GetJsonFromSolid(const G4VSolid* solid){
    json solidJson;

    G4Polyhedron* polyhedron = solid->CreatePolyhedron();
        
    // lambda function to convert double into int then string
    auto format = [](G4double d){
      return std::to_string(static_cast<int>(d));
    };

    // create JSON:
    std::string theJson = "{\"vertices\":[";

    // Get vertices
    G4int numVertices = polyhedron->GetNoVertices();
    // note: vertices are one indexed
    for (G4int i=1; i < numVertices+1; i++) {
      theJson += "[";
      theJson += format(polyhedron->GetVertex(i).x());
      theJson += ",";
      theJson += format(polyhedron->GetVertex(i).y());
      theJson += ",";
      theJson += format(polyhedron->GetVertex(i).z());
      if (i == numVertices){ 
        // the last one
        theJson += "]";
      } else {
        theJson += "],";
      }
    }
    theJson += "],";

    theJson += "\"indices\":[";

    // get indices of edge connections
    // ISSUE HERE" There are only 6 vertices and 
    // the indices go up to 8. They may be 1 indexed..
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
    std::lock_guard<std::mutex> lock(messageQueueMutex);

    return messagesToBeSent.size();
  }

  std::string Server::ViewNextMessage(){
    std::lock_guard<std::mutex> lock(messageQueueMutex);

    if (messagesToBeSent.empty()){
      return "";
    }

    return messagesToBeSent.front();
  }

  void Server::PopNextMessage(){
    std::lock_guard<std::mutex> lock(messageQueueMutex);

    if (messagesToBeSent.empty()){
      std::cout << "Cant pop, message queue empty!" << std::endl;
      return;
    }

    messagesToBeSent.pop();
  }

  int Server::ViewNThreads(){
    std::lock_guard<std::mutex> lock(nThreadsMutex);
  
    return nThreads;
  }

  int Server::ViewNClientsReceived(){
    std::lock_guard<std::mutex> lock(nClientsReceivedMutex);

    return nClientsReceived;
  }

  void Server::AddToNThreads(int num){
    std::lock_guard<std::mutex> lock(nThreadsMutex);

    nThreads += num;
  }

  void Server::AddToNClientsReceived(int num){
    std::lock_guard<std::mutex> lock(nClientsReceivedMutex);

    nClientsReceived += num;
  }

  void Server::SetNClientsReceived(int num){
    std::lock_guard<std::mutex> lock(nClientsReceivedMutex);

    nClientsReceived = num;
  }

  int Server::ViewNNewClients(){
    std::lock_guard<std::mutex> lock(newClientsMutex);

    return newSockets.size();
  }

  void Server::PushNewClient(int sock){
    std::lock_guard<std::mutex> lock(newClientsMutex);

    newSockets.push_back(sock);
  }

  int Server::PopNewClient(){
    std::lock_guard<std::mutex> lock(newClientsMutex);

    int newClient;
    if (newSockets.empty()){
      std::cout << "Can't pop newSockets, it's empty!" << std::endl;
      return -1;
    }

    newClient = newSockets.front();
    newSockets.pop_front();

    return newClient;
  }
} // ! namespace remora
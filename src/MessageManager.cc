#include "MessageManager.hh"

namespace remora {
  MessageManager::MessageManager(){
  }

  bool MessageManager::AddNewClient(int clientID){
    std::lock_guard<std::mutex> lock(clientMapMutex);

    if (clientMap.count(clientID)) return false; 

    clientMap[clientID] = std::queue<std::string>();
    return true;
  }

  bool MessageManager::RemoveClient(int clientID){
    std::lock_guard<std::mutex> lock(clientMapMutex);

    if (!clientMap.count(clientID)) return false;

    clientMap.erase(clientID);
    return true;
  }

  bool MessageManager::ClientExists(int clientID){
    std::lock_guard<std::mutex> lock(clientMapMutex);

    return clientMap.count(clientID);
  }

  bool MessageManager::QueueMessage(int clientID, std::string msg){
    std::lock_guard<std::mutex> lock(clientMapMutex);

    if (!clientMap.count(clientID)) return false;

    clientMap[clientID].push(msg);
    return true;
  }

  bool MessageManager::QueueMessageForAll(std::string msg){
    std::lock_guard<std::mutex> lock(clientMapMutex);

    if (clientMap.empty()) return false;

    for (auto& pair : clientMap) {
      pair.second.push(msg);
    }
    return true;
  }

  std::string MessageManager::GetNextMessage(int clientID){
    std::lock_guard<std::mutex> lock(clientMapMutex);

    if (!clientMap.count(clientID)) return "";
    if (clientMap[clientID].empty()) return "";

    return clientMap[clientID].front();
  }

  bool MessageManager::PopNextMessage(int clientID){
    std::lock_guard<std::mutex> lock(clientMapMutex);

    if (!clientMap.count(clientID)) return false;
    if (clientMap[clientID].empty()) return false;

    clientMap[clientID].pop();
    return true;
  }

  bool MessageManager::MessagesWaiting(int clientID){
    std::lock_guard<std::mutex> lock(clientMapMutex);
    
    if (!clientMap.count(clientID)) return false;

    return !clientMap[clientID].empty();
  }

}
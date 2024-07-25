#ifndef REMORA_MESSAGE_MANAGER_HH
#define REMORA_MESSAGE_MANAGER_HH 1

#include <queue>
#include <string>
#include <unordered_map>

namespace remora {
  
class MessageManager {
public:
  MessageManager();

  void AddNewClient(int clientID);
  void RemoveClient(int clientID);
  bool ClientExists(int clientID);

  bool QueueMessage(int clientID);
  bool QueueMessageForAll();

  std::string GetNextMessage(int clientID);
  void MessageSent(int clientID);

private:
  std::unordered_map<int, std::queue<std::string>> clientMap;

};

} // !namespace remora

#endif;
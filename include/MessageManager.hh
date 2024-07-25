#ifndef REMORA_MESSAGE_MANAGER_HH
#define REMORA_MESSAGE_MANAGER_HH 1

#include <queue>
#include <string>
#include <unordered_map>

namespace remora {
  
class MessageManager {
public:
  MessageManager();

  bool AddNewClient(int clientID);
  bool RemoveClient(int clientID);
  bool ClientExists(int clientID);

  bool QueueMessage(int clientID, std::string msg);
  bool QueueMessageForAll(std::string msg);

  std::string GetNextMessage(int clientID);
  bool PopNextMessage(int clientID);
  bool MessagesWaiting(int clientID);

private:
  std::unordered_map<int, std::queue<std::string>> clientMap;

};

} // !namespace remora

#endif;
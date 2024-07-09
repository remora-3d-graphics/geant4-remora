#include "cross_platform_sockets.hh"

int cp_init(){

  #ifdef _WIN32
  WSADATA wsaData;

  int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
  if (iResult != 0){
    return 1;
  }
  #endif

  return 0;
}

struct addrinfo cp_get_hints(){
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  #ifdef _WIN32
  hints.ai_protocol = IPPROTO_TCP; // WINDOWS
  #else
  hints.ai_flags = AI_PASSIVE; // LINUX
  #endif

  return hints;
}

void cp_close(int socket){
  #ifdef _WIN32
  closesocket(socket);
  #else
  close(socket);
  #endif
}

int cp_get_last_error(){
  int err = 0;
  #ifdef _WIN32
  err = WSAGetLastError();
  #endif

  return err;
}

void cp_cleanup(){
  #ifdef _WIN32
  WSACleanup();
  #endif
}
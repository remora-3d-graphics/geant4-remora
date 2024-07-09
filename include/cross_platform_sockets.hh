// differences:
//
// windows has WSAStartup to link to the .lib
// 
// windows uses UINT_PTR instead of int for sockets,
// however, using an int seems to work.
//
// hints are different. windows only works with ai_protocol defined,
// and linux only works if the client is ai_passive
//
// TODO: freeaddrinfo and getnameinfo are not on windows!
//

#ifndef CROSS_PLATFORM_SOCKETS_HH
#define CROSS_PLATFORM_SOCKETS_HH

// include header files
#ifdef _WIN32

#include <WinSock2.h>
#include <WS2tcpip.h>

#else

#include <cstring>
#include <sys/types.h>
#include <string>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#endif

int cp_init();

struct addrinfo cp_get_hints();

void cp_close(int socket);

int cp_get_last_error();

void cp_cleanup();

#endif // ! CROSS_PLATFORM_SOCKETS_HH
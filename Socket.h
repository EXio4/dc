#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#ifdef _WIN32

#undef UNICODE
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x502
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 


#endif

class Socket {
private:
#ifdef _WIN32
    WSADATA wsaData;
    SOCKET ConnectSocket;
#else
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
#endif
protected:
public:
	Socket(std::string host, int port);
	Socket(const Socket &arg) = delete; // disable copy constructor
	~Socket();
	std::string readLine();
	void write(std::string str);
};


#endif


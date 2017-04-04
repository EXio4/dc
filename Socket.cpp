#include "Socket.h"
#include "SocketException.h"
#include "Pth.h"
#include <string>

#ifdef _WIN32
Socket::Socket(std::string host, int port) {
	ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (iResult != 0) {
		throw new SocketException("WSAStartup failed with error: " + pth::to_string(iResult));
	}
	
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	
	// Resolve the server address and port
	std::string port_s = pth::to_string(port);
	iResult = getaddrinfo(host.c_str(), port_s.c_str(), &hints, &result);
	if ( iResult != 0 ) {
		WSACleanup();
		throw new SocketException("getaddrinfo failed with error: " + pth::to_string(iResult));
	}
	
	// Attempt to connect to an address until one succeeds
	for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {
		
		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
							   ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			iResult = WSAGetLastError();
			WSACleanup();
			throw new SocketException("socket failed with error: " + pth::to_string(iResult));
		}
		
		// Connect to server.
		iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	
	freeaddrinfo(result);
	
	if (ConnectSocket == INVALID_SOCKET) {
		WSACleanup();
		throw new SocketException("Unable to connect to server!");
	}
	
}


Socket::~Socket() {
    closesocket(ConnectSocket);
    WSACleanup();
}


std::string Socket::readLine() {
	std::string s;
	char buffer[1];
	
	int iResult = 0;
	do {
		
		iResult = recv(ConnectSocket, buffer, 1, 0);
		if (iResult < 0) { 
			throw SocketException("recv failed with error: " + pth::to_string(WSAGetLastError()));
		} else if (iResult > 0) {
			if (buffer[0] == '\r') continue;
			if (buffer[0] == '\n') break;
			
			s.push_back(buffer[0]);
		}
		
	} while( iResult > 0 );
	
	return s;
}

void Socket::write(std::string str) {
	str.push_back('\n');
	int iResult = send( ConnectSocket, str.c_str(), (int)str.size(), 0 );
	if (iResult == SOCKET_ERROR) {
		iResult = WSAGetLastError();
		throw new SocketException("send failed with error: " + pth::to_string(iResult));
	}
}

#else

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

Socket::Socket(std::string host, int port) {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        throw new SocketException("ERROR opening socket");
    server = gethostbyname(host.c_str());
    if (server == NULL) {
        throw new SocketException("ERROR, no such host");
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        throw new SocketException("ERROR connecting");
}

Socket::~Socket() {

}

std::string Socket::readLine() {
    std::string s;
    char buffer[1];
    
    int iResult = 0;
    do {
        
        iResult = recv(sockfd, buffer, 1, 0);
        if (iResult < 0) { 
            throw SocketException("recv failed with error: " + pth::to_string(errno));
        } else if (iResult > 0) {
            if (buffer[0] == '\r') continue;
            if (buffer[0] == '\n') break;
            
            s.push_back(buffer[0]);
        }
        
    } while( iResult > 0 );
    
    return s;
}

void Socket::write(std::string str) {
    str.push_back('\n');
    int iResult = send( sockfd, str.c_str(), (int)str.size(), 0);
    if (iResult < 0) {
        iResult = errno;
        throw new SocketException("send failed with error: " + pth::to_string(iResult));
    }
}
#endif

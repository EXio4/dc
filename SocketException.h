#ifndef SOCKETEXCEPTION_H
#define SOCKETEXCEPTION_H

#include <stdexcept>
#include <iostream>


class SocketException : public std::runtime_error {
public:
	SocketException(std::string s) : std::runtime_error(s) {};
	void show() {
		std::cout << "SOCKET " << what() << std::endl;
	}
};

#endif


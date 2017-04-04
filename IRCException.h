#ifndef IRCEXCEPTION_H
#define IRCEXCEPTION_H

#include <stdexcept>
#include <iostream>

class IRCException : public std::runtime_error  {
private:
protected:
public:
	IRCException(std::string s) : std::runtime_error(s) {};
	void show() {
		std::cout << "IRC " << what() << std::endl;
	}
};

#endif


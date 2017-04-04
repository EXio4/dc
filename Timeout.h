#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <stdexcept>
#include <iostream>

class Timeout : public std::runtime_error {
private:
protected:
public:
	Timeout() : std::runtime_error("Timeout") {};
	void show() {
		std::cout << what() << std::endl;
	}
};

#endif


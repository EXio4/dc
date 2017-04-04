#ifndef DC_EXC_H
#define DC_EXC_H

#include <stdexcept>

class DC_Exc : public std::runtime_error {
public:
	DC_Exc(std::string s) : std::runtime_error(s) {};
};

#endif


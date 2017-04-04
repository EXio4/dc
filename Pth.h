#ifndef PTH_H
#define PTH_H
#include <sstream>

namespace pth {
	template<typename T>
	std::string to_string(const T &n) {
		std::ostringstream s;
		s << n;
		return s.str();
	}
};

#endif

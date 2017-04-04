#ifndef IRCMESSAGE_H
#define IRCMESSAGE_H

#include <vector>
#include <string>
#include "IRCException.h"
#include "Pth.h"

class IrcMessage {
private:
	std::vector<std::string> ops;
protected:
public:
	static const int USER    = -30;
	static const int COMMAND = -31;
	static const int TARGET  = -32;
	static const int MESSAGE = -33;
	IrcMessage() {};
	IrcMessage(std::vector<std::string> ops) : ops(ops) {
	};
	~IrcMessage();
	std::string& operator[](int x) {
		switch(x) {
			case USER:
				x = 0;
			break;
			case COMMAND:
				x = 1;
			break;
			case TARGET:
				x = 2;
			break;
			case MESSAGE:
				x = 3;
			break;
		}	
		if (x < 0 || x >= ops.size()) {
			throw IRCException("Out of bounds IRC message address l=" + pth::to_string(x));
		} else {
			return ops[x];
		}
	}

};

#endif


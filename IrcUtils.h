#ifndef IRCUTILS_H
#define IRCUTILS_H

#include <string>
#include "IrcMessage.h"

class IrcUtils {
private:
	static bool is_digit(char x) {
		return (x >= '0' && x <= '9');
	}
protected:
public:
	static IrcMessage parse(std::string s);
	static std::string remove_irc_codes(std::string s);
	static IrcMessage remove_irc_codes(IrcMessage m);
	static std::string nick_from_host(std::string s);
	static IrcMessage work_with_relay(IrcMessage t);
};

#endif


#ifndef COMMANDHANDLER_H
#define COMMANDHANDLER_H

#include <list>
#include <functional>
#include <map>

class IrcClient;

class CommandHandler {
private:
	char prefix;
	std::map<std::string, std::list<std::function<void(IrcClient&, std::string, std::string, std::string)>>> handlers;
protected:
public:
	CommandHandler(char prefix) : prefix(prefix) {};
	void on(std::string x, std::function<void(IrcClient&, std::string, std::string, std::string)> f) {
		handlers[x].push_back(f);
	}
	void operator()(IrcClient& self, std::string nick, std::string channel, std::string cmd);
};

#endif


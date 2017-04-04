#ifndef IRCCLIENT_H
#define IRCCLIENT_H

#include "Socket.h"
#include "IrcUtils.h"
#include <string>
#include <functional>

#define IRC_DEFINITION(name, cmd) \
		private:\
		cmd cb_##name;\
		public:\
		void name(cmd x) {\
			cb_##name = x;\
		}

class IrcClient {
private:
	Socket s;
	IRC_DEFINITION(onPRIVMSG, std::function<void(IrcClient&, std::string, std::string, std::string)>);
	IRC_DEFINITION(onNOTICE, std::function<void(IrcClient&, std::string, std::string, std::string)>);
	IRC_DEFINITION(onCONNECT, std::function<void(IrcClient&)>);
protected:
public:
	IrcClient(std::string host, int port, std::string nick) : s(host, port) {
		s.write("USER irc irc irc :Lame IRC Bot");
		s.write("NICK " + nick);
	}
	~IrcClient();
	void loop();
	
	void write(std::string c) { s.write(c); }
	int privmsg(std::string target, std::string msg);
};
#undef IRC_DEFINITION

#endif


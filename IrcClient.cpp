#include "IrcClient.h"
#include "SocketException.h"
#include "IRCException.h"
#include <iostream>

IrcClient::~IrcClient() {
	
}

void IrcClient::loop() {
	IrcMessage r;
	while (true) {
		std::string x;
		try {
			x = s.readLine();
			std::cout << "Got " << x  << std::endl;
			
			r = IrcUtils::work_with_relay(IrcUtils::remove_irc_codes(IrcUtils::parse(x)));
			
			if (r[0] == "PING") { 
				write("PONG :"+r[1]);
			} else if (r[1] == "376" /* got MOTD */ || r[1] == "422" /* MOTD is missing */) {
				if (cb_onCONNECT) {
					cb_onCONNECT(*this);
				}
			} else if (r[1] == "PRIVMSG") {
				if (cb_onPRIVMSG) {
					cb_onPRIVMSG(*this, IrcUtils::nick_from_host(r[0]), r[2], r[3]);
				}
			} else if (r[2] == "NOTICE") {
				if (cb_onNOTICE) {
					cb_onNOTICE(*this, IrcUtils::nick_from_host(r[0]), r[2], r[3]);
				}
			}
		} catch (IRCException& irc) {
			std::cout << "IRC_Exception [invalid message or something]" << std::endl;
			std::cout << x << std::endl;
			std::cout << "exc " << irc.what() << std::endl;
			std::cout << "----------------" << std::endl;
		} catch (SocketException& sk) {
			std::cout << "Socket Exception" << std::endl;
			std::cout << "exc " << sk.what() << std::endl;
			return;
		}
		
	}
}
int IrcClient::privmsg(std::string target, std::string msg) {
	std::string buff = "";
	int lines = 0;
	for (char i : msg) {
		if (i == '\r') continue;
		if (i == '\n') {
			if (buff != "") {
				write("PRIVMSG "+target+" :"+buff);
				lines++;
				buff = "";
			}
			continue;
		}
		buff.push_back(i);
	}
	if (buff != "") {
		write("PRIVMSG "+target+" :"+buff);
		lines++;
		buff = "";
	}
	return lines;
}

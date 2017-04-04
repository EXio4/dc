#include "CommandHandler.h"

void CommandHandler::operator()(IrcClient& self, std::string nick, std::string channel, std::string cmd) {
	if (cmd.size() >= 1 && cmd[0] == prefix) {
		for (auto k : handlers) {
			if ((1+cmd.size()) >= k.first.size()) {
				bool x = true;
				for (int i=0;i < k.first.size(); i++) {
					char c_i = cmd[i+1];
					if (c_i >= 'A' && c_i <= 'Z') {
						c_i = (c_i - 'A') + 'a';
					}
					x &= k.first[i] == c_i;
				}
				if (x) {
					// matched!1!
					std::string params;
					for (int i=2 + k.first.size(); i < cmd.size(); i++) {
						params.push_back(cmd[i]);
					}
					for (auto h : k.second) {
						h(self, nick, channel, params);
					}
				}
			}
		}
	}
}



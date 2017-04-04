/* i dont know what this is supposed to be, gpl? mit? wtfpl? trolololol? */
#include "IrcClient.h"
#include "irc.h"
#include "CommandHandler.h"

#include <iostream>
#include "dc/dc.h"
#include "dc/dc-proto.h"
#include <iostream>
#include <fstream>

int main (int argc, char *argv[]) {

	std::string host = "irc.6697.eu";
	int         port = 6667;
	std::string nick = "exio_bot";
	std::string ch = "#nyuszika7h";
	//host = "irc.subluminal.net";
	//ch = "#programming";

    if (argc >= 3) {
        host = argv[1];
        ch = argv[2];
    }

	
	IrcClient irc(host, port, nick);
	
	irc.onCONNECT([ch](IrcClient& self) {
		self.write("JOIN "+ch);
	});
	CommandHandler h('$');
	h.on("ping", [](IrcClient& self, std::string nick, std::string channel, std::string params) {
		irc::ostream i (self, channel);
		i << "pong (re: " << nick << ")" << std::endl;
	});
	h.on("run", [](IrcClient& self, std::string nick, std::string channel, std::string params) {
		std::cout << "Running? " << params << std::endl;
		irc::ostream i (self, channel, 4);
		DC dc(i);
		dc_data d = dc.makestring(params.c_str(), params.size());
        int n = 1000;
		try {
			dc.evalstr(n, d);
			i.flush();
		} catch (const Timeout& t) {
			i.clear_lines();
			i << "Timeout" << std::endl;
		} catch (const DC_Exc& e) {
			i.clear_lines();
			i << e.what() << std::endl;
		}
		// THIS will be fixed when we use std::string, fuck it!
//		dc.free_str(&(boost::.get<dc_string>()));
	});
		

	irc.onPRIVMSG(h);

	irc.loop();
	return 0;
}


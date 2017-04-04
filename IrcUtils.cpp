#include "IrcUtils.h"
#include <iostream>

IrcMessage IrcUtils::parse(std::string s) {
	std::vector<std::string> v;
	std::string current;
	bool all=false;
	for (int i=0; i<s.size(); i++) {
		if (!all && s[i] == ' ') {
			v.push_back(current);
			current.clear();
		} else if (!all && i > 0 && s[i]==':' && current.size() == 0) { // : is first element
			all = true;
		} else { 
			current.push_back(s[i]);
		}
	}
	if (current.size() > 0) {
		v.push_back(current);
	}
	
	return IrcMessage(v);
}

std::string IrcUtils::remove_irc_codes(std::string s) {
	std::string r;
	for (int i=0;i<s.size(); i++) {
		if (s[i] == '\x03') {
			if ((i+1) < s.size() && is_digit(s[i+1])) {
				i++;
				if ((i+2) < s.size() && is_digit(s[i+1])) {
					i++;
				}
			}
		} else if (s[i] == '\x02' || s[i] == '\x0F') {
			// ignore clear codes
		} else {
			r.push_back(s[i]);
		}
	}
	return r;
}

IrcMessage IrcUtils::remove_irc_codes(IrcMessage m) {
	if (m[1] == "PRIVMSG" || m[1] == "NOTICE") {
		m[3] = remove_irc_codes(m[3]);
	}
	return m;
}

std::string IrcUtils::nick_from_host(std::string s) {
	std::string nick;
	if (s[0] != ':') { return ""; }
	for (int i=1; i<s.size(); i++) {
		if (s[i] == '!') {
			return nick;
		} else {
			nick.push_back(s[i]);
		}
	}
	return "";
}

IrcMessage IrcUtils::work_with_relay(IrcMessage t) {
	if (t[1] == "PRIVMSG") {
		if (nick_from_host(t[0]) == "t") {
			std::string nick;
			std::string text;
			if (t[3][0] == '<') {
				int i;
				for (i=1; i<t[3].size(); i++) {
					if (t[3][i] == '>') {
						break;
					}
					nick.push_back(t[3][i]);
				}
				i+=2;
				for (;i<t[3].size(); i++) {
					text.push_back(t[3][i]);
				}
				t[0] = ":@"+nick + "!relay@irc.relay";
				t[3] = text;
			}
		} else if (nick_from_host(t[0]) == "relay") {
			std::string nick;
			std::string text;
			int i = 1;
			while (i < t[3].size() && t[3][i] != '\x0F') {
				nick.push_back(t[3][i]);
			}
			i+=4;
			for (;i<t[3].size(); i++) {
				text.push_back(t[3][i]);
			}
			t[0] = ":@"+nick + "!relay@irc.relay";
			t[3] = text;
		}

	}
	return t;
}

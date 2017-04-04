#ifndef IRC_H
#define IRC_H

#include <cstdio>
#include <streambuf>
#include <ostream>
#include "SocketException.h"
#include "IRCException.h"
#include "Timeout.h"

namespace irc {
	class ircbuf : public std::streambuf {
	protected:
		int lines_sent;
	private:
		IrcClient& irc;
		bool enabled;
		std::string target;
		int max_lines;
		char buf[380];
		bool do_flush() {
			std::string s(pbase(), pptr());
			std::ptrdiff_t n = pptr() - pbase();
			pbump(-n);
			try {
				if (!enabled) return false;
				lines_sent += irc.privmsg(target, s);
				if (max_lines > 0 && lines_sent > max_lines) {
					irc.privmsg(target, "Too many lines");
					enabled = false; // hack
					throw Timeout();
					return false;
				}
			} catch (IRCException& e) {
				e.show();
				return false;
			} catch (SocketException& e) {
				e.show();
				return false;
			}
			return true;	
		}
	protected:
		int_type overflow (int_type ch) {
			if (ch != EOF) {
				*pptr() = ch;
				pbump(1);
				if (do_flush()) {
					return ch;
				}
			}
			return EOF;
		}
		int sync() {
			return do_flush() ? 0 : -1;
		}
	public:
		ircbuf(IrcClient& irc, std::string target, int max) : lines_sent(0), irc(irc), target(target), max_lines(max) {
			this->setp(this->buf, this->buf + sizeof(this->buf) - 1);
			enabled = true;
		}
	};
	class ostream : public virtual ircbuf, public std::ostream {
	public:
		ostream(IrcClient& irc, std::string target, int max = -1) : ircbuf(irc, target, max), std::ostream(this) {
		}
		void clear_lines() {
			lines_sent = 0;
		}
	};
};

#endif

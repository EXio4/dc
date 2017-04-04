#ifndef VISITORS_H
#define VISITORS_H

#include "dc.h"
#include "dc-proto.h"

class FreeVar : public boost::static_visitor<> {
private:
	DC dc;
public:
	FreeVar(DC& dc) : dc(dc) {};
	void operator()(UNINITIALIZED) {
		
	}
	void operator()(dc_num num) {
		dc.free_num(&num);
	}
	void operator()(dc_str str) {
		dc.free_str(&str);
	}
	
};
#endif

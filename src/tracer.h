// -*- C++ -*-
#ifndef TRACER_H
#define TRACER_H

#include "debug.h"
#include "LString.h"

class DebugTracer {
public:
	explicit
	DebugTracer(string const & s) : str(s) {
		lyxerr << string(depth, ' ') << "Trace begin : "
		       << str << std::endl;
		++depth;
		
	}
	~DebugTracer() {
		--depth;
		lyxerr << string(depth, ' ') << "Trace end : "
		       << str << std::endl;
	}
private:
	string str;
	static int depth;
};

#endif

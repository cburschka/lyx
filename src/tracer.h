#ifndef TRACER_H
#define TRACER_H

#include "debug.h"
#include "LString.h"

class DebugTracer {
public:
	DebugTracer(string const & s) : str(s) {
		lyxerr << string(depth, ' ') << "Trace begin : " << str << endl;
		++depth;
		
	}
	~DebugTracer() {
		--depth;
		lyxerr << string(depth, ' ') << "Trace end : " << str << endl;
	}
private:
	string str;
	static int depth;
};

#endif

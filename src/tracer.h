// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef TRACER_H
#define TRACER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "debug.h"
#include "LString.h"

///
class DebugTracer {
public:
	///
	explicit
	DebugTracer(string const & s) : str(s) {
		lyxerr << string(depth, ' ') << "Trace begin : "
		       << str << std::endl;
		++depth;
		
	}
	///
	~DebugTracer() {
		--depth;
		lyxerr << string(depth, ' ') << "Trace end : "
		       << str << std::endl;
	}
private:
	///
	string str;
	///
	static int depth;
};

#endif

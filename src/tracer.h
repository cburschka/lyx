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
class Trace {
public:
	///
	explicit
	Trace(string const & s) : str(s) {
		lyxerr << string(depth, ' ') << "TRACE IN: "
		       << str << std::endl;
		depth += 2;
		
	}
	///
	~Trace() {
		depth -= 2;
		lyxerr << string(depth, ' ') << "TRACE OUT: "
		       << str << std::endl;
	}
private:
	///
	string str;
	///
	static int depth;
};

// To avoid wrong usage:
// Trace("BufferView::update");  // wrong
// Trace t("BufferView::update"); // right
// we add this macro:
///
#define Trace(x) unnamed_Trace;
// Tip gotten from Bobby Schmidt's column in C/C++ Users Journal
#endif

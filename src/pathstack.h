// -*- C++ -*-
// lyx-stack.H : definition of PathStack class
//   this file is part of LyX, the High Level Word Processor
//   copyright (C) 1995-1996, Matthias Ettrich and the LyX Team

#ifndef __LYX_STACK_H__
#define __LYX_STACK_H__

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

/// Path stack class definition
class PathStack {
public:
	///
	PathStack() {
		Next = 0;
	}
	///
	PathStack(string const & Path);
	///
	~PathStack();
	///
	int PathPush(string const & Path);
	///
	int PathPop();
private:
	///
	string Path;
	///
	PathStack *Next;
};

// global path stack
extern PathStack lyxPathStack;

/// some global wrapper functions
inline int PathPush(string const & szPath) {
	return lyxPathStack.PathPush(szPath);
}

///
inline int PathPop() { 
	return lyxPathStack.PathPop(); 
}

#endif

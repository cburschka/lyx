// -*- C++ -*-
#ifndef Trans_Decl_h
#define Trans_Decl_h

#include "LString.h"
#include "tex-accent.h"

#include <list>


///
struct Keyexc {
	/// character to make exception
	char c;
	/// exception data
	string data;
	/// Combination with another deadkey
	bool combined;
	/// The accent comined with
	tex_accent accent;
};

///
typedef std::list<Keyexc> KmodException;

///
struct KmodInfo {
	///
	string data;
	///
	tex_accent accent;
	///
	KmodException exception_list;
	///
	KmodInfo();
};

#endif

// -*- C++ -*-
#ifndef Trans_Decl_h
#define Trans_Decl_h

#include <list>

#include "LString.h"
#include "tex-accent.h"

///
struct Keyexc {
	/// character to make exception
	char c;
	/// exception data
	string data;
#if 0
	///
	Keyexc * next;
#endif
	/// Combination with another deadkey
	bool combined;
	/// The accent comined with
	tex_accent accent;
};

#if 0
///
typedef Keyexc * KmodException;
#else
///
typedef std::list<Keyexc> KmodException;
#endif

///
struct KmodInfo {
	///
	string data;
	///
	tex_accent accent;
#if 0
	///
	string allowed;
#endif
	///
	KmodException exception_list;
	///
	KmodInfo();
};

#endif

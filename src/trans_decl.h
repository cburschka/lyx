// -*- C++ -*-
#ifndef Trans_Decl_h
#define Trans_Decl_h

#include "LString.h"
#include "tex-accent.h"

///
struct Keyexc {
	/// character to make exception
	char c;
	/// exception data
	string data;
	///
	Keyexc * next;
	/// Combination with another deadkey
	bool combined;
	/// The accent comined with
	tex_accent accent;
};

///
typedef Keyexc * KmodException;

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

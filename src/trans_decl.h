// -*- C++ -*-
#ifndef Trans_Decl_h
#define Trans_Decl_h

#include "LString.h"
#include "tex-accent.h"

struct Keyexc {
	char c;		/* character to make exception */
	string data;	/* exception data */
	Keyexc * next;
	bool combined;	// Combination with another deadkey
	tex_accent accent;	// The accent combined with
};

///
typedef Keyexc * KmodException;

//
// 
//

struct KmodInfo {
	string data;
	tex_accent accent;
	string allowed;
	KmodException exception_list;    
	KmodInfo();
};

#endif

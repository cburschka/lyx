// -*- C++ -*-
#ifndef _Trans_Decl_h_
#define _Trans_Decl_h_

#include "LString.h"
#include "tex-accent.h"

struct Keyexc {
	char c;		/* character to make exception */
	LString data;	/* exception data */
	Keyexc *next;
	bool combined;	// Combination with another deadkey
	tex_accent accent;	// The accent combined with
};

///
typedef Keyexc *KmodException;

//
// 
//

struct KmodInfo {
	LString data;
	tex_accent accent;
	LString allowed;
	KmodException exception_list;    
	
	KmodInfo(const KmodInfo&);
	KmodInfo();
	
	KmodInfo& operator=(const KmodInfo&);
};

#endif

// -*- C++ -*-
#ifndef _Chset_h_
#define _Chset_h_

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

///
class CharacterSet {
public:
	///
	CharacterSet();
	///
	~CharacterSet();
	
	///
	bool loadFile(const LString&);
	///
	LString getName();
	///
	bool encodeString(LString&);
private:
	///
	LString name_;
	
	///
	struct Cdef {
		///
		unsigned char ic;
		///
		LString str;
		///
		Cdef *next;
	};
	
	///
	Cdef *map_;
	///
	void freeMap();
};
#endif

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
	bool loadFile(const string&);
	///
	string getName();
	///
	bool encodeString(string&);
private:
	///
	string name_;
	
	///
	struct Cdef {
		///
		unsigned char ic;
		///
		string str;
		///
		Cdef *next;
	};
	
	///
	Cdef *map_;
	///
	void freeMap();
};
#endif

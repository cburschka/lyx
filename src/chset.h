// -*- C++ -*-
#ifndef CHSET_H
#define CHSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>
#include "LString.h"

using std::map;

///
class CharacterSet {
public:
	///
	bool loadFile(string const &);
	///
	string const & getName() const;
	///
	bool encodeString(string &) const;
private:
	///
	string name_;
	///
	typedef map<string, unsigned char> Cdef;
	///
	Cdef map_;
};
#endif

// -*- C++ -*-
#ifndef CHSET_H
#define CHSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>
#include <utility>

using std::map;
using std::pair;

#include "LString.h"

///
class CharacterSet {
public:
	///
	bool loadFile(string const &);
	///
	string const & getName() const;
	///
	pair<bool, int> encodeString(string &) const;
private:
	///
	string name_;
	///
	typedef map<string, unsigned char> Cdef;
	///
	Cdef map_;
};
#endif

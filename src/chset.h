// -*- C++ -*-
#ifndef CHSET_H
#define CHSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>
using std::map;

#include <utility>
using std::pair;
using std::make_pair;  // may also be put in chset.C

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

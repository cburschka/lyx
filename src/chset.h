// -*- C++ -*-
#ifndef CHSET_H
#define CHSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>
#include <utility>

#include "LString.h"

///
class CharacterSet {
public:
	///
	bool loadFile(string const &);
	///
	string const & getName() const;
	///
	std::pair<bool, int> encodeString(string const &) const;
private:
	///
	string name_;
	///
	typedef std::map<string, unsigned char> Cdef;
	///
	Cdef map_;
};
#endif

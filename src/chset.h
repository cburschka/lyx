// -*- C++ -*-
/**
 * \file chset.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 */

#ifndef CHSET_H
#define CHSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>
#include <utility>

#include "LString.h"

/// a class for mapping char strings such as "\^{A}" to the integer value
class CharacterSet {
public:
	/**
	 * initialise this charset from the given .cdef file
	 * param charset the charset to look for
	 *
	 * Finds a .cdef file corresponding to the named charset
	 * and parses it. This function is only intended to be
	 * called once.
	 */
	bool loadFile(string const & charset);
	/// return the name of the current charset
	string const & getName() const;
	/**
	 * Return the encoded charset value of the given string.
	 *
	 * The bool value is false if an encoding could not be found
	 * in this charset, and true otherwise.
	 */
	std::pair<bool, int> const encodeString(string const &) const;
private:
	/// charset name
	string name_;
	///
	typedef std::map<string, unsigned char> Cdef;
	/// mapping from string representation to encoded value
	Cdef map_;
};
#endif

// -*- C++ -*-
/**
 * \file chset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CHSET_H
#define CHSET_H

#include <map>
#include <utility>
#include <string>


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
	bool loadFile(std::string const & charset);
	/// return the name of the current charset
	std::string const & getName() const;
	/**
	 * Return the encoded charset value of the given string.
	 *
	 * The bool value is false if an encoding could not be found
	 * in this charset, and true otherwise.
	 */
	std::pair<bool, int> const encodeString(std::string const &) const;
private:
	/// charset name
	std::string name_;
	///
	typedef std::map<std::string, unsigned char> Cdef;
	/// mapping from string representation to encoded value
	Cdef map_;
};
#endif

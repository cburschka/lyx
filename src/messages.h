// -*- C++ -*-
* \file messages.h
* This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef MESSAGES_H
#define MESSAGES_H

#include "LString.h"

#include <locale>

///
class Messages {
public:
	///
	typedef std::messages<char>::catalog catalog;
	///
	Messages(string const & l, string const & dir);
	///
	~Messages();
	///
	string const get(string const & msg) const;
	///
	string const & lang() const {
		return lang_;
	}
	///
	string const & localedir() const {
		return localedir_;
	}
private:
	///
	string lang_;
	///
	string localedir_;
	///
	std::locale loc_gl;
	///
	std::messages<char> const & mssg_gl;
	///
	catalog cat_gl;
};

#endif

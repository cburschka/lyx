// -*- C++ -*-
/**
 * \file FloatList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef FLOATLIST_H
#define FLOATLIST_H

#include <map>

#include "LString.h"
#include "Floating.h"

///
class FloatList {
public:
	///
	typedef std::map<string, Floating> List;
	///
	typedef List::const_iterator const_iterator;
	///
	FloatList();
	///
	const_iterator begin() const;
	///
	const_iterator end() const;
	///
	void newFloat(Floating const & fl);
	///
	string const defaultPlacement(string const & t) const;
	///
	bool typeExist(string const & t) const;
	///
	Floating const & getType(string const & t) const;
	///
	void erase(string const & t);
	///
	const_iterator operator[](string const & t) const;
private:
	///
	List list;
};

#endif

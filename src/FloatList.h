// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998-2000 The LyX Team.
 *
 * ======================================================
 */

#ifndef FLOATLIST_H
#define FLOATLIST_H

#ifdef __GNUG__
#pragma interface
#endif

#include <map>

#include "LString.h"
#include "Floating.h"

///
class FloatList {
public:
	///
	typedef std::map<string, Floating> List;
	///
	FloatList();
	///
	void newFloat(Floating const & fl);
	///
	string defaultPlacement(string const & t) const;
	///
	bool typeExist(string const & t) const;
	///
	Floating const & getType(string const & t) const;
private:
	///
	List list;
};

///
extern FloatList floatList;

#endif

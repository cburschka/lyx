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
	FloatList() {
		// Insert the latex builtin float-types
		Floating table;
		table.type = "table";
		table.placement = "";
		table.ext = "lot";
		table.within = "";
		table.style = "";
		table.name = "";
		table.builtin = true;
		list[table.type] = table;
		Floating figure;
		figure.type = "figure";
		figure.placement = "";
		figure.ext = "lof";
		figure.within = "";
		figure.style = "";
		figure.name = "";
		figure.builtin = true;
		list[figure.type] = figure;
		// And we add algorithm too since LyX has
		// supported that for a long time
		Floating algorithm;
		algorithm.type = "algorithm";
		algorithm.placement = "htbp";
		algorithm.ext = "loa";
		algorithm.within = "";
		algorithm.style = "ruled";
		algorithm.name = "Algorithm";
		algorithm.builtin = false;
		list[algorithm.type] = algorithm;
	}
	///
	void newFloat(Floating const & fl) {
		list[fl.type] = fl;
	}
	///
	string defaultPlacement(string const & t) const {
		List::const_iterator cit = list.find(t);
		if (cit != list.end())
			return (*cit).second.placement;
		return string();
	}
	
private:
	///
	List list;
};

extern FloatList floatList;

#endif

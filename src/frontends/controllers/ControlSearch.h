// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlSearch.h
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#ifndef CONTROLSEARCH_H
#define CONTROLSEARCH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialogs.h"

/** A controller for Search dialogs.
 */
class ControlSearch : public ControlDialog<ControlConnectBD> {
public:
	///
	ControlSearch(LyXView &, Dialogs &);
   
	/// Searches occurence of string
	void find(string const & search,
		  bool casesensitive, bool matchword, bool forward) const;

	/// Replaces occurence of string
	void replace(string const & search, string const & replace,
		     bool casesensitive, bool matchword, bool all) const;

private:
	/// not needed.
	virtual void apply() {}
};

#endif // CONTROLSEARCH_H

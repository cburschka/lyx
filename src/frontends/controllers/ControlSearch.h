// -*- C++ -*-
/**
 * \file ControlSearch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLSEARCH_H
#define CONTROLSEARCH_H


#include "ControlDialog_impl.h"

/** A controller for Search dialogs.
 */
class ControlSearch : public ControlDialogBD {
public:
	///
	ControlSearch(LyXView &, Dialogs &);

	/// Searches occurence of string
	void find(std::string const & search,
		  bool casesensitive, bool matchword, bool forward);

	/// Replaces occurence of string
	void replace(std::string const & search, std::string const & replace,
		     bool casesensitive, bool matchword,
		     bool forward, bool all);
private:
	/// not needed.
	virtual void apply() {}
};

#endif // CONTROLSEARCH_H

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

#include "Dialog.h"


/** A controller for Search dialogs.
 */
class ControlSearch : public Dialog::Controller {
public:
	ControlSearch(Dialog &);

	virtual bool initialiseParams(std::string const &) { return true; }
	virtual void clearParams() {}
	virtual void dispatchParams() {}
	virtual bool isBufferDependent() const { return true; }

	/// Searches occurence of string
	void find(std::string const & search,
		  bool casesensitive, bool matchword, bool forward);

	/// Replaces occurence of string
	void replace(std::string const & search, std::string const & replace,
		     bool casesensitive, bool matchword,
		     bool forward, bool all);
};

#endif // CONTROLSEARCH_H

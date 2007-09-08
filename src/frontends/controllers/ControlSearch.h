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

namespace lyx {
namespace frontend {

/** A controller for Search dialogs.
 */
class ControlSearch : public Controller {
public:
	ControlSearch(Dialog &);

	virtual bool initialiseParams(std::string const &) { return true; }
	virtual void clearParams() {}
	virtual void dispatchParams() {}
	virtual bool isBufferDependent() const { return true; }

	/// Searches occurence of string
	void find(docstring const & search,
		  bool casesensitive, bool matchword, bool forward);

	/// Replaces occurence of string
	void replace(docstring const & search, docstring const & replace,
		     bool casesensitive, bool matchword,
		     bool forward, bool all);
};

} // namespace frontend
} // namespace lyx

#endif // CONTROLSEARCH_H

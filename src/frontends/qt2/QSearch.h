// -*- C++ -*-
/**
 * \file QSearch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QSEARCH_H
#define QSEARCH_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlSearch;
class QSearchDialog;


///
class QSearch
	: public Qt2CB<ControlSearch, Qt2DB<QSearchDialog> >
{
public:
	///
	friend class QSearchDialog;
	///
	QSearch();
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents() {}
	/// build the dialog
	virtual void build_dialog();

	void find(string const & str, bool casesens,
		  bool words, bool backwards);

	void replace(string const & findstr, string const & replacestr,
		bool casesens, bool words, bool all);
};

#endif // QSEARCH_H

// -*- C++ -*-
/**
 * \file QSearch.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
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
	QSearch(ControlSearch &);

private:
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents() {};
	/// build the dialog
	virtual void build_dialog();


	void find(string const & str, bool casesens, bool words, bool backwards);

	void replace(string const & findstr, string const & replacestr,
		bool casesens, bool words, bool all);

};

#endif // QSEARCH_H

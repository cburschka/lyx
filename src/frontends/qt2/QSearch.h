// -*- C++ -*-
/**
 * \file QSearch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QSEARCH_H
#define QSEARCH_H

#include "QDialogView.h"

class ControlSearch;
class QSearchDialog;


///
class QSearch
	: public QController<ControlSearch, QView<QSearchDialog> >
{
public:
	///
	friend class QSearchDialog;
	///
	QSearch(Dialog &);
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents() {}
	/// build the dialog
	virtual void build_dialog();

	void find(std::string const & str, bool casesens,
		  bool words, bool backwards);

	void replace(std::string const & findstr,
		     std::string const & replacestr,
		     bool casesens, bool words, bool backwards, bool all);
};

#endif // QSEARCH_H

// -*- C++ -*-
/**
 * \file QToc.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTOC_H
#define QTOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"
#include "toc.h"

class ControlToc;
class QTocDialog;

class QToc :
	public Qt2CB<ControlToc, Qt2DB<QTocDialog> >
{
public:
	QToc();

	friend class QTocDialog;

private:
	/// update the listview
	void updateToc(int newdepth);

	/// update the float types
	void updateType();

	/// select an entry
	void select(string const & text);

	/// set the depth
	void set_depth(int depth);

	virtual void apply() {};

	/// update dialog
	virtual void update_contents();

	/// build dialog
	virtual void build_dialog();

	/// the toc list
	toc::Toc toclist;

	/// depth of list shown
	int depth_;
};

#endif // QTOC_H

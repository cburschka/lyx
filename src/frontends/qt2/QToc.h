// -*- C++ -*-
/**
 * \file QToc.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QTOC_H
#define QTOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

#include "ControlToc.h"

class ControlToc;
class QTocDialog;

class QToc :
	public Qt2CB<ControlToc, Qt2DB<QTocDialog> >
{
public:
	QToc(ControlToc &);

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
	Buffer::SingleList toclist;

	/// depth of list shown
	int depth_;
};

#endif // QTOC_H

// -*- C++ -*-
/**
 * \file QBibitem.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QBIBITEM_H
#define QBIBITEM_H

#include "Qt2Base.h"

class ControlBibitem;
class QBibitemDialog;

class QBibitem :
	public Qt2CB<ControlBibitem, Qt2DB<QBibitemDialog> >
{
	friend class QBibitemDialog;

public:
	QBibitem();

protected:
	virtual bool isValid();

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QBIBITEM_H

// -*- C++ -*-
/**
 * \file QBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QBIBITEM_H
#define QBIBITEM_H

#ifdef __GNUG__
#pragma interface
#endif

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

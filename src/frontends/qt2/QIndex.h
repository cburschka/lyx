// -*- C++ -*-
/**
 * \file QIndex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QINDEX_H
#define QINDEX_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlIndex;
class QIndexDialog;


class QIndex :
	public Qt2CB<ControlIndex, Qt2DB<QIndexDialog> >
{
public:
	friend class QIndexDialog;

	QIndex();
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

#endif // QINDEX_H

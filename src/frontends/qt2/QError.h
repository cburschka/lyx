// -*- C++ -*-
/**
 * \file QError.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QERROR_H
#define QERROR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlError;
class QErrorDialog;


class QError
	: public Qt2CB<ControlError, Qt2DB<QErrorDialog> >
{
public:
	friend class QErrorDialog;

	QError();
private:
	/// Apply changes
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QERROR_H

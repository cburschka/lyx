// -*- C++ -*-
/**
 * \file QError.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QERROR_H
#define QERROR_H

#include "Qt2Base.h"

class ControlError;
class QErrorDialog;
class Dialogs;

class QError :
	public Qt2CB<ControlError, Qt2DB<QErrorDialog> >
{
	friend class QErrorDialog;

public:
	QError(ControlError &, Dialogs &);

private:
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QERROR_H

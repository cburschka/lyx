// -*- C++ -*-
/**
 * \file QTabularCreate.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QTABULARCREATE_H
#define QTABULARCREATE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlTabularCreate;
class QTabularCreateDialog;


///
class QTabularCreate
	: public Qt2CB<ControlTabularCreate, Qt2DB<QTabularCreateDialog> >
{
public:
	///
	friend class QTabularCreateDialog;
	///
	QTabularCreate();

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents() {};
	/// build the dialog
	virtual void build_dialog();
};

#endif // QTABULARCREATE_H

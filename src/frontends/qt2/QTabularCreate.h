// -*- C++ -*-
/**
 * \file QTabularCreate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
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

// -*- C++ -*-
/**
 * \file QChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QCHANGES_H
#define QCHANGES_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlChanges;
class QChangesDialog;

class QChanges
	: public Qt2CB<ControlChanges, Qt2DB<QChangesDialog> >
{
public:
	friend class QChangesDialog;

	QChanges();

	void accept();

	void reject();

	void next();

private:
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents() {};
	/// build the dialog
	virtual void build_dialog();
};

#endif // QCHANGES_H

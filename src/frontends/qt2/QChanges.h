// -*- C++ -*-
/**
 * \file QChanges.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QCHANGES_H
#define QCHANGES_H

#include "QDialogView.h"

class ControlChanges;
class QChangesDialog;

class QChanges
	: public QController<ControlChanges, QView<QChangesDialog> >
{
public:
	friend class QChangesDialog;

	QChanges(Dialog &);

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

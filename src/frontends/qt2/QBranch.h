// -*- C++ -*-
/**
 * \file QBranch.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QBRANCH_H
#define QBRANCH_H

#include "QDialogView.h"


class ControlBranch;
class QBranchDialog;

/** This class provides a QT implementation of the Branch Dialog.
 */
class QBranch : public QController<ControlBranch, QView<QBranchDialog> >
{
public:
	friend class QBranchDialog;

	/// Constructor
	QBranch(Dialog &);
private:
	///  Apply changes
	virtual void apply();
	/// Build the dialog
	virtual void build_dialog();
	/// Update dialog before showing it
	virtual void update_contents();
};

#endif // QBRANCH_H

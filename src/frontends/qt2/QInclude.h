// -*- C++ -*-
/**
 * \file QInclude.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QINCLUDE_H
#define QINCLUDE_H


#include "QDialogView.h"


class ControlInclude;
class QIncludeDialog;

///
class QInclude : public QController<ControlInclude, QView<QIncludeDialog> >
{
public:
	///
	friend class QIncludeDialog;
	///
	QInclude(Dialog &);
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	/// load the file
	void load();

	/// browse for a file
	void browse();
};

#endif // QINCLUDE_H

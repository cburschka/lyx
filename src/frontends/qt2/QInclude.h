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

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlInclude;
class QIncludeDialog;

///
class QInclude
	: public Qt2CB<ControlInclude, Qt2DB<QIncludeDialog> >
{
public:
	///
	friend class QIncludeDialog;
	///
	QInclude();
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

// -*- C++ -*-
/**
 * \file QExternal.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QEXTERNAL_H
#define QEXTERNAL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlExternal;
class QExternalDialog;

class QExternal
	: public Qt2CB<ControlExternal, Qt2DB<QExternalDialog> >
{
public:
	friend class QExternalDialog;

	QExternal();
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	/// get the right helptext
	string const & helpText();
};

#endif // QEXTERNAL_H

// -*- C++ -*-
/**
 * \file QPrefs.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QPREFS_H
#define QPREFS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"
#include "ControlPrefs.h"

class QPrefsDialog;

class QPrefs
	: public Qt2CB<ControlPrefs, Qt2DB<QPrefsDialog> >
{
public:

	friend class QPrefsDialog;

	QPrefs();
 
private:
	/// Apply changes
	virtual void apply();
 
	/// update (do we need this?)
	virtual void update_contents();

	/// build the dialog
	virtual void build_dialog();
};

#endif // QPREFS_H

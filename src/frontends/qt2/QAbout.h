// -*- C++ -*-
/**
 * \file QAbout.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef FORMABOUT_H
#define FORMABOUT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class QAboutDialog;
class ControlAboutlyx;

class QAbout
	: public Qt2CB<ControlAboutlyx, Qt2DB<QAboutDialog> >
{
public:
	QAbout();
private:
	/// not needed
	virtual void apply() {}
	/// not needed
	virtual void update_contents() {}
	// build the dialog
	virtual void build_dialog();
};

#endif // FORMABOUT_H

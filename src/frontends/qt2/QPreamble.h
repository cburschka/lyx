// -*- C++ -*-
/**
 * \file QPreamble.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QPREAMBLE_H
#define QPREAMBLE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlPreamble;
class QPreambleDialog;

///
class QPreamble
	: public Qt2CB<ControlPreamble, Qt2DB<QPreambleDialog> >
{
public:
	///
	friend class QPreambleDialog;
	///
	QPreamble();

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QPREAMBLE_H

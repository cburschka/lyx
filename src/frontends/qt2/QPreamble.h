// -*- C++ -*-
/**
 * \file QPreamble.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
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
	QPreamble(ControlPreamble &);

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QPREAMBLE_H

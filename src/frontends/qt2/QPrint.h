// -*- C++ -*-
/**
 * \file QPrint.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 * \author Edwin Leuven, leuven@fee.uva.nl
 */

#ifndef QPRINT_H
#define QPRINT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlPrint;
class QPrintDialog;
class Dialogs;

///
class QPrint
	: public Qt2CB<ControlPrint, Qt2DB<QPrintDialog> >
{
public:
	///
	friend class QPrintDialog;
	///
	QPrint(ControlPrint &, Dialogs &);

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QPRINT_H

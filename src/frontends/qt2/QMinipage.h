// -*- C++ -*-
/**
 * \file QMinipage.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QMINIPAGE_H
#define QMINIPAGE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlMinipage;
class QMinipageDialog;

///
class QMinipage
	: public Qt2CB<ControlMinipage, Qt2DB<QMinipageDialog> >
{
public:
	///
	friend class QMinipageDialog;
	///
	QMinipage(ControlMinipage &, Dialogs &);

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QMINIPAGE_H

// -*- C++ -*-
/**
 * \file QMinipage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
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
	QMinipage();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QMINIPAGE_H

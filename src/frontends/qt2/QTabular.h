// -*- C++ -*-
/**
 * \file QTabular.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Kalle Dalheimer
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QTABULAR_H
#define QTABULAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlTabular;
class QTabularDialog;


class QTabular :
	public Qt2CB<ControlTabular, Qt2DB<QTabularDialog> >
{
public:
	friend class QTabularDialog;

	QTabular();
 
protected:
	virtual bool isValid();
 
private:
	/// We can't use this ...
	virtual void apply() {}
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QTABULAR_H

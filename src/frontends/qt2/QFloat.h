// -*- C++ -*-
/**
 * \file QFloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QFLOAT_H
#define QFLOAT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlFloat;
class QFloatDialog;

///
class QFloat
	: public Qt2CB<ControlFloat, Qt2DB<QFloatDialog> >
{
public:
	///
	friend class QFloatDialog;
	///
	QFloat();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QFLOAT_H

// -*- C++ -*-
/**
 * \file QFloat.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Edwin Leuven <leuven@fee.uva.nl>
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
	QFloat(ControlFloat &);
private:
	/// Apply changes
	virtual void apply();
	/// update 
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QFLOAT_H





// -*- C++ -*-
/**
 * \file QPrint.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Edwin Leuven
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QPRINT_H
#define QPRINT_H


#include "Qt2Base.h"

class ControlPrint;
class QLPrintDialog;

///
class QPrint
	: public Qt2CB<ControlPrint, Qt2DB<QLPrintDialog> >
{
public:
	///
	friend class QLPrintDialog;
	///
	QPrint();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QPRINT_H

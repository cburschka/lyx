// -*- C++ -*-
/**
 * \file QERT.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QERT_H
#define QERT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "Qt2Base.h"

class ControlERT;
class QERTDialog;

class QERT
	: public Qt2CB<ControlERT, Qt2DB<QERTDialog> >
{
public:
	friend class QERTDialog;

	QERT();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QERT_H

// -*- C++ -*-
/**
 * \file QERT.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QERT_H
#define QERT_H

#include "Qt2Base.h"

class ControlERT;
class QERTDialog;

class QERT :
	public Qt2CB<ControlERT, Qt2DB<QERTDialog> > 
{
	friend class QERTDialog;
 
public: 
	QERT(ControlERT &);

private: 
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QERT_H

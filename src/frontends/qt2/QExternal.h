// -*- C++ -*-
/**
 * \file QExternal.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QEXTERNAL_H
#define QEXTERNAL_H

#include "Qt2Base.h"

class ControlExternal;
class QExternalDialog;

class QExternal :
	public Qt2CB<ControlExternal, Qt2DB<QExternalDialog> > 
{
	friend class QExternalDialog;
 
public: 
	QExternal(ControlExternal &);

protected:
	virtual bool isValid();
 
private: 
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
 
	/// get the right helptext
	string const & helpText();
};

#endif // QEXTERNAL_H

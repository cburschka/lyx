// -*- C++ -*-
/**
 * \file QBibtex.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QBIBTEX_H
#define QBIBTEX_H

#include "Qt2Base.h"

class ControlBibtex;
class QBibtexDialog;

class QBibtex :
	public Qt2CB<ControlBibtex, Qt2DB<QBibtexDialog> >
{
	friend class QBibtexDialog;

public:
	QBibtex();

protected:
	virtual bool isValid();

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QBIBTEX_H

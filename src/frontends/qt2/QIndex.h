// -*- C++ -*-
/**
 * \file QIndex.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef QINDEX_H
#define QINDEX_H

#include "Qt2Base.h"

class ControlIndex;
class QIndexDialog;

class QIndex :
	public Qt2CB<ControlIndex, Qt2DB<QIndexDialog> >
{
	friend class QIndexDialog;

public:
	QIndex(ControlIndex &);

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

#endif // QINDEX_H

// -*- C++ -*-
/**
 * \file QGraphics.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QGRAPHICS_H
#define QGRAPHICS_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insets/insetgraphics.h"
#include "Qt2Base.h"

class ControlGraphics;
class QGraphicsDialog;

///
class QGraphics
	: public Qt2CB<ControlGraphics, Qt2DB<QGraphicsDialog> >
{
public:
	///
	friend class QGraphicsDialog;
	///
	QGraphics(ControlGraphics &, Dialogs &);

protected:
	virtual bool isValid();

private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();

	/// browse for a file
	void browse();
	/// get bounding box from file
	void get();
};

#endif // QGRAPHICS_H

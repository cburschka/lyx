// -*- C++ -*-
/**
 * \file QGraphics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QGRAPHICS_H
#define QGRAPHICS_H

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
	QGraphics();
protected:
	virtual bool isValid();
private:
	/// Apply changes
	virtual void apply();
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
	/// get bounding box from file
	void getBB();

	/// Store the LaTeX names for the rotation origins.
	std::vector<string> origin_ltx;

};

#endif // QGRAPHICS_H

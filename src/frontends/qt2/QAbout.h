// -*- C++ -*-
/**
 * \file QAbout.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author Kalle Dalheimer <kalle@klaralvdalens-datakonsult.se>
 */

#ifndef FORMABOUT_H
#define FORMABOUT_H

#include "Qt2Base.h"
#include "boost/utility.hpp"

class Dialogs;
class LyXView;
class QAboutDialog;
class ControlAboutlyx;

class QAbout
	: public Qt2CB<ControlAboutlyx, Qt2DB<QAboutDialog> >
{
public:
	QAbout(ControlAboutlyx &, Dialogs &);

private:
	/// not needed
	virtual void apply() {}
	/// not needed
	virtual void update_contents() {}
	// build the dialog
	virtual void build_dialog();
};

#endif // FORMABOUT_H

// -*- C++ -*-
/**
 * \file QShowFile.h
 * Copyright 2001 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef QSHOWFILE_H
#define QSHOWFILE_H

#include "Qt2Base.h"

class ControlShowFile;
class QShowFileDialog;


class QShowFile :
	public Qt2CB<ControlShowFile, Qt2DB<QShowFileDialog> >
{
	friend class QShowFileDialog;

public:
	QShowFile();

private:
	/// Apply changes
	virtual void apply() {};
	/// update
	virtual void update_contents();
	/// build the dialog
	virtual void build_dialog();
};

#endif // QSHOWFILE_H

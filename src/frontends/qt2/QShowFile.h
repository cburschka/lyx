// -*- C++ -*-
/**
 * \file QShowFile.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QSHOWFILE_H
#define QSHOWFILE_H

#ifdef __GNUG__
#pragma interface
#endif

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

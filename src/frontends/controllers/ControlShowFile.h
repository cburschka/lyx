// -*- C++ -*-
/**
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \file ControlShowFile.h
 * \author Herbert Voss 
 *
 * Full author contact details are available in file CREDITS
 */
#ifndef CONTROLSHOWFILE_H
#define CONTROLSHOWFILE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "LString.h"

/** A controller for the ShowFile dialog. */

class ControlShowFile : public ControlDialogBI {
public:
	///
	ControlShowFile(LyXView &, Dialogs &);
	///
	virtual void showFile(string const &);
	///
	string getFileContents();
	///
	string getFileName();

private:
	/// not needed.
	virtual void apply() {}
	///
	string filename_;
};

#endif // CONTROLSHOWFILE_H

// -*- C++ -*-
/**
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \file ControlShowFile.h
 * \author Herbert Voss <voss@perce.de>
 */
#ifndef CONTROLSHOWFILE_H
#define CONTROLSHOWFILE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"

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

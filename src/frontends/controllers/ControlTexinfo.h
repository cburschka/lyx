// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlTexinfo.h
 * \author Herbert Voss <voss@perce.de>
 */

#ifndef CONTROLTEXINFO_H
#define CONTROLTEXINFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"

/** A controller for Texinfo dialogs. */

class ControlTexinfo : public ControlDialogBI {
public:
	/// the file extensions
	enum texFileSuffix {cls, sty, bst};
	///
	ControlTexinfo(LyXView &, Dialogs &);
	///
	void viewFile(string const filename) const;
	///
	void help() const;
	///
	void rescanStyles() const;
	///
	void runTexhash() const;
	///
	string const getContents(texFileSuffix type, bool withPath) const;


private:
	///
	virtual void apply() {}
};

#endif // CONTROLTEXINFO_H

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

#include "ControlDialogs.h"

/** A controller for Texinfo dialogs. */

class ControlTexinfo : public ControlDialog<ControlConnectBI> {
public:
	///
	ControlTexinfo(LyXView &, Dialogs &);
	///
	void viewFile(string const filename);
	///
	void help();
	///
	void rescanStyles();
	///
	void runTexhash();

private:
	///
	virtual void apply() {}
};

#endif // CONTROLTEXINFO_H

// -*- C++ -*-
/**
 * \file ControlTexinfo.h
 * See the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLTEXINFO_H
#define CONTROLTEXINFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialog_impl.h"
#include "LString.h"

/** A controller for Texinfo dialogs. */

class ControlTexinfo : public ControlDialogBI {
public:
	/// the file extensions
	enum texFileSuffix {cls, sty, bst};
	///
	ControlTexinfo(LyXView &, Dialogs &);
	/// show contents af a file
	void viewFile(string const filename) const;
	/// show all classoptions
	string const getClassOptions(string const & filename) const;
	/// build new cls bst sty - lists
	void rescanStyles() const;
	/// build new bst sty cls lists
	void runTexhash() const;
	/// read filecontents
	string const getContents(texFileSuffix type, bool withPath) const;


private:
	///
	virtual void apply() {}
};

#endif // CONTROLTEXINFO_H

// -*- C++ -*-
/**
 * \file ControlTexinfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef CONTROLTEXINFO_H
#define CONTROLTEXINFO_H


#include "Dialog.h"

/** A controller for Texinfo dialogs. */

class ControlTexinfo : public Dialog::Controller {
public:
	///
	ControlTexinfo(Dialog &);
	///
	virtual bool initialiseParams(string const &) { return true; }
	///
	virtual void clearParams() {}
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return false; }


	/// the file extensions
	enum texFileSuffix {cls, sty, bst};
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

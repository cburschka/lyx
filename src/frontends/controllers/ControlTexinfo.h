// -*- C++ -*-
/**
 * \file ControlTexinfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLTEXINFO_H
#define CONTROLTEXINFO_H


#include "Dialog.h"
#include "tex_helpers.h"

/** A controller for Texinfo dialogs. */

class ControlTexinfo : public Dialog::Controller {
public:
	///
	ControlTexinfo(Dialog &);
	/// Nothing to initialise in this case.
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
	void viewFile(string const & filename) const;
	/// show all classoptions
	string const getClassOptions(string const & filename) const;
private:
	///
	virtual void apply() {}
};


/** Fill \c contents from one of the three texfiles.
 *  Each entry in the file list is returned as a pair<name_with_path, name_only>
 */
void getTexFileList(ControlTexinfo::texFileSuffix type,
		    std::vector<string> & contents);

#endif // CONTROLTEXINFO_H

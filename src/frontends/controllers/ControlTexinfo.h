// -*- C++ -*-
/**
 * \file ControlTexinfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef CONTROLTEXINFO_H
#define CONTROLTEXINFO_H


#include "Dialog.h"
#include "frontend_helpers.h"

namespace lyx {
namespace frontend {

/** A controller for Texinfo dialogs. */

class ControlTexinfo : public Controller {
public:
	///
	ControlTexinfo(Dialog &);
	/// Nothing to initialise in this case.
	virtual bool initialiseParams(std::string const &) { return true; }
	///
	virtual void clearParams() {}
	///
	virtual void dispatchParams() {}
	///
	virtual bool isBufferDependent() const { return false; }


	/// the file extensions
	enum texFileSuffix {cls, sty, bst};
	/// show contents af a file
	void viewFile(std::string const & filename) const;
	/// show all classoptions
	std::string const getClassOptions(std::string const & filename) const;
	/// return file type as string
	std::string const getFileType(ControlTexinfo::texFileSuffix type) const;
private:
	///
	virtual void apply() {}
};


/** Fill \c contents from one of the three texfiles.
 *  Each entry in the file list is returned as a pair<name_with_path, name_only>
 */
void getTexFileList(ControlTexinfo::texFileSuffix type,
		    std::vector<std::string> & contents, bool withPath);

} // namespace frontend
} // namespace lyx

#endif // CONTROLTEXINFO_H

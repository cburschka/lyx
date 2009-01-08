// -*- C++ -*-
/**
 * \file LyXVC.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_VC_H
#define LYX_VC_H

#include <boost/scoped_ptr.hpp>

#include <string>


namespace lyx {

namespace support { class FileName; }

class VCS;
class Buffer;

/** Version Control for LyX.
    This is the class giving the verison control features to LyX. It is
    intended to support different kinds of version control.
    The support in LyX is based loosely upon the version control in GNU Emacs,
    but is not as extensive as that one. See Extended Manual for a simple
    tutorial and manual for the use of the version control system in LyX.

    LyXVC use this algorithm when it searches for VC files:
    for RCS it searches for <filename>,v and RCS/<filename>,v similarly
    CVS/Entries for cvs and .svn/entries. By doing this there doesn't need to be any
    special support for VC in the lyx format, and this is especially good
    when the lyx format will be a subset of LaTeX.
*/
class LyXVC {
public:
	///
	LyXVC();
	///
	~LyXVC();
	/** Not a good name perhaps. This function should be called whenever
	  LyX loads a file. This function then checks for a master VC file (for
	  RCS this is *,v or RCS/ *,v ; for CVS this is CVS/Entries and .svn/entries
	  for SVN) if this file or entry is found, the loaded file is assumed to be
	  under controll by VC, and the appropiate actions is taken.
	  Returns true if the file is under control by a VCS.
	  */
	bool file_found_hook(support::FileName const & fn);

	/** This function should be run when a file is requested for loading,
	  but it does not exist. This function will then check for a VC master
	  file with the same name (see above function). If this exists the
	  user should be asked if he/her wants to checkout a version for
	  viewing/editing. Returns true if the file is under control by a VCS
	  and the user wants to view/edit it.
	  */
	static bool file_not_found_hook(support::FileName const & fn);

	///
	void setBuffer(Buffer *);

	/// Register the document as an VC file.
	bool registrer();

	/// Unlock and commit changes. Returns log.
	std::string checkIn();
	/// Does the current VC supports this operation?
	bool checkInEnabled();

	/// Lock/update and prepare to edit document. Returns log.
	std::string checkOut();
	/// Does the current VC supports this operation?
	bool checkOutEnabled();

	/// Revert to last version
	void revert();

	/// Undo last check-in.
	void undoLast();
	/// Does the current VC supports this operation?
	bool undoLastEnabled();

	/**
	 * Generate a log file and return the filename.
	 * It is the caller's responsibility to remove the
	 * file after use.
	 */
	const std::string getLogFile() const;

	///
	void toggleReadOnly();

	/// Is the document under administration by VCS?
	bool inUse();

	/// Returns the version number.
	//std::string const & version() const;
	/// Returns the version number.
	std::string const versionString() const;

	/// Returns the userid of the person who has locked the doc.
	std::string const & locker() const;

private:
	///
	Buffer * owner_;

	///
	boost::scoped_ptr<VCS> vcs;
};


} // namespace lyx

#endif

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


	// std::string used as a return value in functions below are
	// workaround to defer messages to be displayed in UI. If message()
	// is used directly, message string is immediately overwritten
	// by the next multiple messages on the top of the processed dispatch
	// machinery.

	/// Unlock and commit changes. Returns log.
	std::string checkIn();
	/// Does the current VC supports this operation?
	bool checkInEnabled() const;

	/// Lock/update and prepare to edit document. Returns log.
	std::string checkOut();
	/// Does the current VC supports this operation?
	bool checkOutEnabled() const;

	/// Synchronize the whole archive with repository
	std::string repoUpdate();
	/// Does the current VC supports this operation?
	bool repoUpdateEnabled() const;

	/**
	 * Toggle locking property of the edited file,
	 * i.e. whether the file uses locking mechanism.
	 */
	std::string lockingToggle();
	/// Does the current VC support this operation?
	bool lockingToggleEnabled() const;

	/// Revert to last version
	void revert();

	/// Undo last check-in.
	void undoLast();
	/// Does the current VC supports this operation?
	bool undoLastEnabled() const;

	/**
	 * Generate a log file and return the filename.
	 * It is the caller's responsibility to remove the
	 * file after use.
	 */
	const std::string getLogFile() const;

	/**
	 * We do not support this generally. In RCS/SVN file read-only flag
	 * is often connected with locking state and one has to be careful to
	 * keep things in synchro once we would allow user to toggle
	 * read-only flags.
	 */
	void toggleReadOnly();

	/// Is the document under administration by VCS?
	bool inUse() const;

	/// FIXME Either rename or kill, we have revisionInfo now.
	/// Returns the version number.
	//std::string const & version() const;
	/// Returns the version number.
	std::string const versionString() const;

	/**
	 * Returns the userid of the person who has locked the doc.
	 * FIXME This property is descendant from the original VCS
	 * and used in RCS for user id or "unlocked" strings.
	 * It would be problem to use this under SVN since getting
	 * the locker would need connection to server any time we
	 * load document. SVN currently (mis)uses this as a signal
	 * for locking state, as we do not have API for vcstatus().
	 * The RCS code for user id parsing is working but we don't display
	 * it anywhere. One possibility is to provide proper vcstatus()
	 * interface and kill the whole locker thing.
	 */
	std::string const & locker() const;

	// type of the revision information
	enum RevisionInfo {
		File = 1,
		Tree = 2,
		Author = 3,
		Date = 4,
		Time = 5
	};

	/**
	 * Return revision info specified by the argument.
	 * Its safe to call it regardless VCS is in usage or this
	 * info is (un)available. Returns empty string in such a case.
	 */
	std::string revisionInfo(RevisionInfo const info);

private:
	///
	Buffer * owner_;

	///
	boost::scoped_ptr<VCS> vcs;
};


} // namespace lyx

#endif

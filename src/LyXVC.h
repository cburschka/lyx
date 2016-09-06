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

#include "support/unique_ptr.h"

#include <string>


namespace lyx {

namespace support { class FileName; }

class VCS;
class Buffer;

/** Version Control for LyX.
    This is the class giving the version control features to LyX. It is
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
	/// Return status of a command
	enum CommandResult {
		Cancelled,    ///< command was cancelled
		ErrorBefore,  ///< error before executing command
		ErrorCommand, ///< error while executing command
		VCSuccess     ///< command was executed successfully
	};
	///
	LyXVC();
	/// Name of the underlying VCS
	std::string vcname() const;
	/// Is \p fn under version control?
	static bool fileInVC(support::FileName const & fn);
	/** Not a good name perhaps. This function should be called whenever
	  LyX loads a file. This function then checks for a master VC file (for
	  RCS this is *,v or RCS/ *,v ; for CVS this is CVS/Entries and .svn/entries
	  for SVN) if this file or entry is found, the loaded file is assumed to be
	  under control by VC, and the appropiate actions is taken.
	  Returns true if the file is under control by a VCS.
	  */
	bool file_found_hook(support::FileName const & fn);

	/** Is \p fn under version control?
	  This function should be run when a file is requested for loading,
	  but it does not exist. This function will then check for a VC master
	  file with the same name (see above function). If this exists the
	  user should be asked if he/her wants to checkout a version for
	  viewing/editing. Returns true if the file is under control by a VCS
	  and the user wants to view/edit it.
	  */
	static bool file_not_found_hook(support::FileName const & fn);

	///
	void setBuffer(Buffer *);

	/// Register the document as a VC file.
	bool registrer();


	// std::string used as a return value in functions below are
	// workaround to defer messages to be displayed in UI. If message()
	// is used directly, message string is immediately overwritten
	// by the next multiple messages on the top of the processed dispatch
	// machinery.

	///
	std::string rename(support::FileName const &);
	/// Does the current VC support this operation?
	bool renameEnabled() const;
	///
	std::string copy(support::FileName const &);
	/// Does the current VC support this operation?
	bool copyEnabled() const;

	/// Unlock and commit changes.
	/// \p log is non-empty on success and may be empty on failure.
	CommandResult checkIn(std::string & log);
	/// Does the current VC support this operation?
	bool checkInEnabled() const;
	/// Should a log message be provided for next checkin?
	bool isCheckInWithConfirmation() const;

	/// Lock/update and prepare to edit document. Returns log.
	std::string checkOut();
	/// Does the current VC support this operation?
	bool checkOutEnabled() const;

	/// Synchronize the whole archive with repository
	std::string repoUpdate();
	/// Does the current VC support this operation?
	bool repoUpdateEnabled() const;

	/**
	 * Toggle locking property of the edited file,
	 * i.e. whether the file uses locking mechanism.
	 */
	std::string lockingToggle();
	/// Does the current VC support this operation?
	bool lockingToggleEnabled() const;

	/// Revert to last version
	bool revert();

	/// Undo last check-in.
	void undoLast();
	/// Does the current VC support this operation?
	bool undoLastEnabled() const;
	/**
	 * Prepare revision rev of the file into newly created temporary file
	 * and save the filename into parameter f.
	 * Parameter rev can be either revision number or negative number
	 * which is interpreted as how many revision back from the current
	 * one do we want. rev=0 is reserved for the last (committed) revision.
	 * We need rev to be string, since in various VCS revision is not integer.
	 * If RCS addressed by a single number, it is automatically used
	 * as the last number in the whole revision specification (it applies
	 * for retrieving normal revisions (rev>0) or backtracking (rev<0).
	 */
	bool prepareFileRevision(std::string const & rev, std::string & f);
	/// Does the current VC support this operation?
	bool prepareFileRevisionEnabled();

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
	std::string toggleReadOnly();

	/// Is the document under administration by VCS?
	/// returns false for unregistered documents in a path managed by VCS
	bool inUse() const;

	/// Returns the RCS + version number for messages
	std::string const versionString() const;

	/**
	 * Returns whether we use locking for the given file.
	 */
	bool locking() const;

	// type of the revision information
	enum RevisionInfo {
		Unknown = 0,
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
	std::string revisionInfo(RevisionInfo const info) const;

private:
	///
	Buffer * owner_;

	///
	unique_ptr<VCS> vcs;
};


} // namespace lyx

#endif

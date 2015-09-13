// -*- C++ -*-
/**
 * \file VCBackend.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Pavel Sanda
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef VC_BACKEND_H
#define VC_BACKEND_H

#include "support/FileName.h"

#include <string>
#include <vector>

#include "LyXVC.h"


namespace lyx {

class Buffer;

/// A simple version control system interface
class VCS {
public:
	/// the status of the managed file
	enum VCStatus {
		UNLOCKED,
		LOCKED,
		NOLOCKING,
	};

	VCS(Buffer * b) : vcstatus(NOLOCKING), owner_(b) {}
	virtual ~VCS() {}

	/// register a file for version control
	virtual void registrer(std::string const & msg) = 0;
	/// can this operation be processed in the current VCS?
	virtual bool renameEnabled() = 0;
	/// rename a file. Return non-empty log on success, empty log on failure.
	virtual std::string rename(support::FileName const &, std::string const &) = 0;
	/// can this operation be processed in the current VCS?
	virtual bool copyEnabled() = 0;
	/// copy a file. Return non-empty log on success, empty log on failure.
	virtual std::string copy(support::FileName const &, std::string const &) = 0;
	/// check in the current revision.
        /// \p log is non-empty on success and may be empty on failure.
	virtual LyXVC::CommandResult
	checkIn(std::string const & msg, std::string & log) = 0;
	/// can this operation be processed in the current VCS?
	virtual bool checkInEnabled() = 0;
	/// should a log message provided for next checkin?
	virtual bool isCheckInWithConfirmation() = 0;
	/// check out for editing, returns log
	virtual std::string checkOut() = 0;
	/// can this operation be processed in the current VCS?
	virtual bool checkOutEnabled() = 0;
	/// synchronize with repository, returns log
	virtual std::string repoUpdate() = 0;
	/// can this operation be processed in the current VCS?
	virtual bool repoUpdateEnabled() = 0;
	/// toggle locking property of the file
	virtual std::string lockingToggle() = 0;
	/// can this operation be processed in the current VCS?
	virtual bool lockingToggleEnabled() = 0;
	/// revert current edits
	virtual bool revert() = 0;
	/// should a confirmation before revert requested?
	virtual bool isRevertWithConfirmation() = 0;
	/**
	 * Merge the current with the previous version
	 * in a reverse patch kind of way, so that the
	 * result is to revert the last changes.
	 */
	virtual void undoLast() = 0;
	/// can this operation be processed in the current VCS?
	virtual bool undoLastEnabled() = 0;
	/**
	 * getLog - read the revision log into the given file
	 * @param fname file name to read into
	 */
	virtual void getLog(support::FileName const &) = 0;
	/// return the current version description
	virtual std::string const versionString() const = 0;
	/// return the owning buffer
	Buffer * owner() const { return owner_; }
	/// return the lock status of this file
	VCStatus status() const { return vcstatus; }
	/// do we need special handling for read-only toggling?
	/// (also used for check-out operation)
	virtual bool toggleReadOnlyEnabled() = 0;
	/// Return revision info specified by the argument.
	virtual std::string revisionInfo(LyXVC::RevisionInfo const info) = 0;
	/// can this operation be processed in the current VCS?
	virtual bool prepareFileRevision(std::string const & rev, std::string & f) = 0;
	/// can this operation be processed in the current VCS?
	virtual bool prepareFileRevisionEnabled() = 0;

	/// Check the directory of file and all parent directories
	/// for the existence of repository-info like .git or .svn
	static bool checkparentdirs(support::FileName const & file, std::string const & vcsdir);
	
protected:
	/// parse information from the version file
	virtual void scanMaster() = 0;

	/// Prepare a version identifier suitable for RCS and CVS.
	/// If needed converts last or relative number to the absolute revision.
	bool makeRCSRevision(std::string const &version, std::string &revis) const;
	
	/// GUI container for doVCCommandCall
	int doVCCommand(std::string const & cmd, support::FileName const & path, bool reportError = true);
	/**
	 * doVCCommandCall - call out to the version control utility
	 * @param cmd the command to execute
	 * @param path the path from which to execute
	 * @return exit status
	 */
	static int doVCCommandCall(std::string const & cmd, support::FileName const & path);

	/**
	 * The master VC file. For RCS this is *,v or RCS/ *,v. master should
	 * have full path.
	 */
	support::FileName master_;

	/// The status of the VC controlled file.
	VCStatus vcstatus;

	/// The buffer using this VC
	Buffer * const owner_;
};


///
class RCS : public VCS {
public:

	explicit
	RCS(support::FileName const & m, Buffer * b);

	/// return the revision file for the given file, if found
	static support::FileName const findFile(support::FileName const & file);

	/// get file from repo, the caller must ensure that it does not exist locally
	static bool retrieve(support::FileName const & file);

	virtual void registrer(std::string const & msg);

	virtual bool renameEnabled();

	virtual std::string rename(support::FileName const &, std::string const &);

	virtual bool copyEnabled();

	virtual std::string copy(support::FileName const &, std::string const &);

	virtual LyXVC::CommandResult
	checkIn(std::string const & msg, std::string & log);

	virtual bool checkInEnabled();

	virtual bool isCheckInWithConfirmation();

	virtual std::string checkOut();

	virtual bool checkOutEnabled();

	virtual std::string repoUpdate();

	virtual bool repoUpdateEnabled();

	virtual std::string lockingToggle();

	virtual bool lockingToggleEnabled();

	virtual bool revert();

	virtual bool isRevertWithConfirmation();

	virtual void undoLast();

	virtual bool undoLastEnabled();

	virtual void getLog(support::FileName const &);

	virtual std::string const versionString() const {
		return "RCS: " + version_;
	}

	virtual bool toggleReadOnlyEnabled();

	virtual std::string revisionInfo(LyXVC::RevisionInfo const info);

	virtual bool prepareFileRevision(std::string const & rev, std::string & f);

	virtual bool prepareFileRevisionEnabled();

protected:
	virtual void scanMaster();
private:
	bool getRevisionInfo();
	/**
	 * The version of the VC file. I am not sure if this can be a
	 * string or if it must be a float/int.
	 */
	std::string version_;
	/// The user currently keeping the lock on the VC file (or "Unlocked").
	std::string locker_;
	/// Cache for revision info.
	std::string rev_date_cache_;
	///
	std::string rev_time_cache_;
	///
	std::string rev_author_cache_;
};


///
class CVS : public VCS {
public:
	///
	explicit
	CVS(support::FileName const & m, Buffer * b);

	/// return the revision file for the given file, if found
	static support::FileName const findFile(support::FileName const & file);

	/// get file from repo, the caller must ensure that it does not exist locally
	static bool retrieve(support::FileName const & file);

	virtual void registrer(std::string const & msg);

	virtual bool renameEnabled();

	virtual std::string rename(support::FileName const &, std::string const &);

	virtual bool copyEnabled();

	virtual std::string copy(support::FileName const &, std::string const &);

	virtual LyXVC::CommandResult
	checkIn(std::string const & msg, std::string & log);

	virtual bool checkInEnabled();

	virtual bool isCheckInWithConfirmation();

	virtual std::string checkOut();

	virtual bool checkOutEnabled();

	virtual std::string repoUpdate();

	virtual bool repoUpdateEnabled();

	virtual std::string lockingToggle();

	virtual bool lockingToggleEnabled();

	virtual bool isRevertWithConfirmation();

	virtual bool revert();

	virtual void undoLast();

	virtual bool undoLastEnabled();

	virtual void getLog(support::FileName const &);

	/// Check for messages in cvs output. 
	/// Returns conflict line.
	std::string scanLogFile(support::FileName const & f, std::string & status);

	virtual std::string const versionString() const {
		return "CVS: " + version_;
	}

	virtual bool toggleReadOnlyEnabled();

	virtual std::string revisionInfo(LyXVC::RevisionInfo const info);

	virtual bool prepareFileRevision(std::string const & rev, std::string & f);

	virtual bool prepareFileRevisionEnabled();

protected:
	virtual void scanMaster();
	/// the mode of operation for some VC commands
	enum OperationMode {
		Directory = 0,
		File = 1
	};
	/// possible status values of file
	enum CvsStatus {
		UpToDate = 0,
		LocallyModified = 1,
		LocallyAdded = 2,
		NeedsMerge = 3,
		NeedsCheckout = 4,
		NoCvsFile = 5,
		StatusError = 6
	};

private:
	// revision number from scanMaster
	std::string version_;

	/**
	 * doVCCommandWithOutput
	 * - call out to the version control utility
	 * - it is able to collect output in a file
	 * @param cmd the command to execute
	 * @param path the path from which to execute
	 * @param output the path where to store output
	 * @param reportError display of low level error message dialog
	 * @return exit status
	 */
	int doVCCommandWithOutput(std::string const & cmd,
			support::FileName const & path,
			support::FileName const & output,
			bool reportError = true);
	static int doVCCommandCallWithOutput(std::string const & cmd,
			support::FileName const & path,
			support::FileName const & output);
						
	/// return the quoted pathname if Directory or filename if File
	virtual std::string const getTarget(OperationMode opmode) const;
	/// collect the diff of file or directory against repository
	/// result is placed in temporary file
	void getDiff(OperationMode opmode, support::FileName const & tmpf);
	/// make the file ready for editing:
	/// save a copy in CVS/Base and change file permissions to rw if needed
	virtual int edit();
	/// revert the edit operation
	virtual int unedit();
	/// retrieve repository changes into working copy
	virtual int update(OperationMode opmode, support::FileName const & tmpf);
	/// check readonly state for file
	/// assume true when file is writable
	virtual bool isLocked() const;
	/// query and parse the cvs status of file
	virtual CvsStatus getStatus();
	/// convert enum to string
	virtual docstring toString(CvsStatus status) const;

	/// cache the info values of current file revision
	/// author, date and time of commit
	std::string rev_author_cache_;
	std::string rev_date_cache_;
	std::string rev_time_cache_;
	/// fills the cache values, returns true if successfull.
	void getRevisionInfo();
	bool have_rev_info_;
};


///
class SVN : public VCS {
public:
	///
	explicit
	SVN(support::FileName const & m, Buffer * b);

	/// return the revision file for the given file, if found
	static support::FileName const findFile(support::FileName const & file);

	/// get file from repo, the caller must ensure that it does not exist locally
	static bool retrieve(support::FileName const & file);

	virtual void registrer(std::string const & msg);

	virtual bool renameEnabled();

	virtual std::string rename(support::FileName const &, std::string const &);

	virtual bool copyEnabled();

	virtual std::string copy(support::FileName const &, std::string const &);

	virtual LyXVC::CommandResult
	checkIn(std::string const & msg, std::string & log);

	virtual bool checkInEnabled();

	virtual bool isCheckInWithConfirmation();

	virtual std::string checkOut();

	virtual bool checkOutEnabled();

	virtual std::string repoUpdate();

	virtual bool repoUpdateEnabled();

	virtual std::string lockingToggle();

	virtual bool lockingToggleEnabled();

	virtual bool revert();

	virtual bool isRevertWithConfirmation();

	virtual void undoLast();

	virtual bool undoLastEnabled();

	virtual void getLog(support::FileName const &);

	virtual std::string const versionString() const {
		return "SVN: " + rev_file_cache_;
	}

	virtual bool toggleReadOnlyEnabled();

	virtual std::string revisionInfo(LyXVC::RevisionInfo const info);

	virtual bool prepareFileRevision(std::string const & rev, std::string & f);

	virtual bool prepareFileRevisionEnabled();

protected:
	virtual void scanMaster();
	/// Check for messages in svn output. Returns error.
	std::string scanLogFile(support::FileName const & f, std::string & status);
	/// checks locking policy and setup locked_mode_
	bool checkLockMode();
	/// is the loaded file locked?
	bool isLocked() const;
	/// acquire/release write lock for the current file
	bool fileLock(bool lock, support::FileName const & tmpf, std::string & status);
	/// Check in files \p f with log \p msg
	LyXVC::CommandResult checkIn(std::vector<support::FileName> const & f,
	                             std::string const & msg, std::string & log);

private:
	/// is the loaded file under locking policy?
	bool locked_mode_;
	/**
	 * Real code for obtaining file revision info. Fills all file-related caches
	 * and returns true if successfull.
	 * "?" is stored in rev_file_cache_ as a signal if request for obtaining info
	 * was already unsuccessful.
	 */
	bool getFileRevisionInfo();
	/// cache for file revision number, "?" if already unsuccessful, isNumber==true
	std::string rev_file_cache_;
	/// cache for author of last commit
	std::string rev_author_cache_;
	/// cache for date of last commit
	std::string rev_date_cache_;
	/// cache for time of last commit
	std::string rev_time_cache_;
	/// fills rev_tree_cache_, returns true if successfull.
	bool getTreeRevisionInfo();
	/// cache for tree revision number, "?" if already unsuccessful
	std::string rev_tree_cache_;
};


/**
 * Very basic git support:
 * Remote repos are completely ignored, only the local tree is considered.
 * How push and pull could be integrated with the LyX VCS interface needs
 * to be discussed.
 */
class GIT : public VCS {
public:
	///
	explicit
	GIT(support::FileName const & m, Buffer * b);

	/// return the revision file for the given file, if found
	static support::FileName const findFile(support::FileName const & file);

	/// get file from repo, the caller must ensure that it does not exist locally
	static bool retrieve(support::FileName const & file);

	virtual void registrer(std::string const & msg);

	virtual bool renameEnabled();

	virtual std::string rename(support::FileName const &, std::string const &);

	virtual bool copyEnabled();

	virtual std::string copy(support::FileName const &, std::string const &);

	virtual LyXVC::CommandResult
	checkIn(std::string const & msg, std::string & log);

	virtual bool checkInEnabled();

	virtual bool isCheckInWithConfirmation();

	virtual std::string checkOut();

	virtual bool checkOutEnabled();

	virtual std::string repoUpdate();

	virtual bool repoUpdateEnabled();

	virtual std::string lockingToggle();

	virtual bool lockingToggleEnabled();

	virtual bool revert();

	virtual bool isRevertWithConfirmation();

	virtual void undoLast();

	virtual bool undoLastEnabled();

	virtual void getLog(support::FileName const &);

	virtual std::string const versionString() const {
		return "GIT: ?";
	}

	virtual bool toggleReadOnlyEnabled();

	virtual std::string revisionInfo(LyXVC::RevisionInfo const info);

	virtual bool prepareFileRevision(std::string const & rev, std::string & f);

	virtual bool prepareFileRevisionEnabled();

protected:
	virtual void scanMaster();
	/// Check for messages in svn output. Returns error.
	std::string scanLogFile(support::FileName const & f, std::string & status);
	/// Check in files \p f with log \p msg
	LyXVC::CommandResult checkIn(std::vector<support::FileName> const & f,
	                             std::string const & msg, std::string & log);

private:
	/**
	 * Real code for obtaining file revision info. Fills all file-related caches
	 * and returns true if successfull.
	 * "?" is stored in rev_file_cache_ as a signal if request for obtaining info
	 * was already unsuccessful.
	 */
	bool getFileRevisionInfo();
	/// cache for file revision number, "?" if already unsuccessful, isNumber==true
	std::string rev_file_cache_;
	/// cache for author of last commit
	std::string rev_author_cache_;
	/// cache for date of last commit
	std::string rev_date_cache_;
	/// cache for time of last commit
	std::string rev_time_cache_;
	/// fills rev_tree_cache_, returns true if successfull.
	bool getTreeRevisionInfo();
	/// cache for tree revision number, "?" if already unsuccessful
	std::string rev_tree_cache_;
};

} // namespace lyx

#endif // VCBACKEND_H

// -*- C++ -*-
/**
 * \file VCBackend.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef VC_BACKEND_H
#define VC_BACKEND_H

#include "support/FileName.h"

#include <string>


namespace lyx {

class Buffer;

/// A simple version control system interface
class VCS {
public:
	/// the status of the managed file
	enum VCStatus {
		UNLOCKED,
		LOCKED,
		NOLOCKING
	};

	virtual ~VCS() {}

	/// register a file for version control
	virtual void registrer(std::string const & msg) = 0;
	/// check in the current revision, returns log
	virtual std::string checkIn(std::string const & msg) = 0;
	// can be this operation processed in the current RCS?
	virtual bool checkInEnabled() = 0;
	/// check out for editing, returns log
	virtual std::string checkOut() = 0;
	// can be this operation processed in the current RCS?
	virtual bool checkOutEnabled() = 0;
	/// revert current edits
	virtual void revert() = 0;
	/// FIXME
	virtual void undoLast() = 0;
	// can be this operation processed in the current RCS?
	virtual bool undoLastEnabled() = 0;
	/**
	 * getLog - read the revision log into the given file
	 * @param fname file name to read into
	 */
	virtual void getLog(support::FileName const &) = 0;
	/// return the current version description
	virtual std::string const versionString() const = 0;
	/// return the current version
	std::string const & version() const { return version_; }
	/// return the user who has locked the file
	std::string const & locker() const { return locker_; }
	/// set the owning buffer
	void owner(Buffer * b) { owner_ = b; }
	/// return the owning buffer
	Buffer * owner() const { return owner_; }
	/// return the lock status of this file
	VCStatus status() const { return vcstatus; }
	/// do we need special handling for read-only toggling?
	/// (also used for check-out operation)
	virtual bool toggleReadOnlyEnabled() = 0;
protected:
	/// parse information from the version file
	virtual void scanMaster() = 0;

	// GUI container for doVCCommandCall
	int doVCCommand(std::string const & cmd, support::FileName const & path);
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

	/**
	 * The version of the VC file. I am not sure if this can be a
	 * string or if it must be a float/int.
	 */
	std::string version_;

	/// The user currently keeping the lock on the VC file.
	std::string locker_;
	/// The buffer using this VC
	Buffer * owner_;
};


///
class RCS : public VCS {
public:

	explicit
	RCS(support::FileName const & m);

	/// return the revision file for the given file, if found
	static support::FileName const findFile(support::FileName const & file);

	static void retrieve(support::FileName const & file);

	virtual void registrer(std::string const & msg);

	virtual std::string checkIn(std::string const & msg);

	virtual bool checkInEnabled();

	virtual std::string checkOut();

	virtual bool checkOutEnabled();

	virtual void revert();

	virtual void undoLast();

	virtual bool undoLastEnabled();

	virtual void getLog(support::FileName const &);

	virtual std::string const versionString() const {
		return "RCS: " + version_;
	}

	virtual bool toggleReadOnlyEnabled();

protected:
	virtual void scanMaster();
};


///
class CVS : public VCS {
public:
	///
	explicit
	CVS(support::FileName const & m, support::FileName const & f);

	/// return the revision file for the given file, if found
	static support::FileName const findFile(support::FileName const & file);

	virtual void registrer(std::string const & msg);

	virtual std::string checkIn(std::string const & msg);

	virtual bool checkInEnabled();

	virtual std::string checkOut();

	virtual bool checkOutEnabled();

	virtual void revert();

	virtual void undoLast();

	virtual bool undoLastEnabled();

	virtual void getLog(support::FileName const &);

	virtual std::string const versionString() const {
		return "CVS: " + version_;
	}

	virtual bool toggleReadOnlyEnabled();

protected:
	virtual void scanMaster();

private:
	support::FileName file_;
};


///
class SVN : public VCS {
public:
	///
	explicit
	SVN(support::FileName const & m, support::FileName const & f);

	/// return the revision file for the given file, if found
	static support::FileName const findFile(support::FileName const & file);

	virtual void registrer(std::string const & msg);

	virtual std::string checkIn(std::string const & msg);

	virtual bool checkInEnabled();

	virtual std::string checkOut();

	virtual bool checkOutEnabled();

	virtual void revert();

	virtual void undoLast();

	virtual bool undoLastEnabled();

	virtual void getLog(support::FileName const &);

	virtual std::string const versionString() const {
		return "SVN: " + version_;
	}

	virtual bool toggleReadOnlyEnabled();

protected:
	virtual void scanMaster();
	/// Check for messages in svn output. Returns error.
	std::string scanLogFile(support::FileName const & f, std::string & status);
	/// checks locking policy and setup locked_mode_
	bool checkLockMode();
	/// is the loaded file locked?
	bool isLocked() const;
	/// acquire/release write lock for the current file
	void fileLock(bool lock, support::FileName const & tmpf, std::string & status);

private:
	support::FileName file_;
	/// is the loaded file under locking policy?
	bool locked_mode_;
};

} // namespace lyx

#endif // VCBACKEND_H

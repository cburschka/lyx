// -*- C++ -*-
/**
 * \file vc-backend.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes
 */


#ifndef VC_BACKEND_H
#define VC_BACKEND_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class Buffer;

/// A simple version control system interface
class VCS {
public:
	/// the status of the managed file
	enum VCStatus {
		UNLOCKED,
		LOCKED
	};

	virtual ~VCS() {}

	/// register a file for version control
	virtual void registrer(string const & msg) = 0;
	/// check in the current revision
	virtual void checkIn(string const & msg) = 0;
	/// check out for editing
	virtual void checkOut() = 0;
	/// revert current edits
	virtual void revert() = 0;
	/// FIXME
	virtual void undoLast() = 0;
	/**
	 * getLog - read the revision log into the given file
	 * @param fname file name to read into
	 */
	virtual void getLog(string const &) = 0;
	/// return the current version description
	virtual string const versionString() const = 0;
	/// return the current version
	string const & version() const {
		return version_;
	}
	/// return the user who has locked the file
	string const & locker() const { return locker_; }
	/// set the owning buffer
	void owner(Buffer * b) { owner_ = b; }
	/// return the owning buffer
	Buffer * owner() const { return owner_; }
	/// return the lock status of this file
	VCStatus status() const { return vcstatus; }
protected:
	/// parse information from the version file
	virtual void scanMaster() = 0;

	/**
	 * doVCCommand - call out to the version control utility
	 * @param cmd the command to execute
	 * @param path the path from which to execute
	 * @return exit status
	 */
	static int doVCCommand(string const & cmd, string const & path);

	/**
	 * The master VC file. For RCS this is *,v or RCS/ *,v. master should
	 * have full path.
	 */
	string master_;

	/// The status of the VC controlled file.
	VCStatus vcstatus;

	/**
	 * The version of the VC file. I am not sure if this can be a
	 * string or if it must be a float/int.
	 */
	string version_;

	/// The user currently keeping the lock on the VC file.
	string locker_;
	/// The buffer using this VC
	Buffer * owner_;
};


///
class RCS : public VCS {
public:

	explicit
	RCS(string const & m);

	/// return the revision file for the given file, if found
	static string const find_file(string const & file);

	static void retrieve(string const & file);

	virtual void registrer(string const & msg);

	virtual void checkIn(string const & msg);

	virtual void checkOut();

	virtual void revert();

	virtual void undoLast();

	virtual void getLog(string const &);

	virtual string const versionString() const {
		return "RCS: " + version_;
	}

protected:
	virtual void scanMaster();
};


///
class CVS : public VCS {
public:
	///
	explicit
	CVS(string const & m, string const & f);

	/// return the revision file for the given file, if found
	static string const find_file(string const & file);

	virtual void registrer(string const & msg);

	virtual void checkIn(string const & msg);

	virtual void checkOut();

	virtual void revert();

	virtual void undoLast();

	virtual void getLog(string const &);

	virtual string const versionString() const {
		return "CVS: " + version_;
	}

protected:
	virtual void scanMaster();

private:
	string file_;
};
#endif // VCBACKEND_H

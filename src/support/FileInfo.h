// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#ifndef FILE_INFO_H
#define FILE_INFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include <ctime>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <boost/utility.hpp>
#include "LString.h"

/** Use objects of this class to get information about files.
 */
class FileInfo : public noncopyable {
public:
	///
	FileInfo();
	
	/** Get information about file.
	    If link is true, the information is about the link itself, not
	    the file that is obtained by tracing the links. */
	explicit
	FileInfo(string const & path, bool link = false);

	/// File descriptor
	explicit
	FileInfo(int fildes);

	/// Query a new file
	FileInfo & newFile(string const & path, bool link = false);
	
	/// Query a new file descriptor
        FileInfo & newFile(int fildes);
	
	/// Returns a character describing file type (ls -F)
	char const * typeIndicator() const;
	
	/// File protection mode
	mode_t getMode() const;

	/// Constructs standard mode string (ls style)
	void modeString(char * szString) const;
	
	/// returns a letter describing a file type (ls style)
	char typeLetter() const;
	
	/// builds 'rwx' string describing file access rights
	void flagRWX(mode_t i, char * szString) const;
	
	/// updates mode string to match suid/sgid/sticky bits
	void setSticky(char * szString) const;
	
	///
	time_t getModificationTime() const;
	
	///
	time_t getAccessTime() const;
	
	///
	time_t getStatusChangeTime() const;
	
	/// Total file size in bytes
	off_t getSize() const;
	
	/// Number of hard links
	nlink_t getNumberOfLinks() const;
	
	/// User ID of owner
	uid_t getUid() const;
	
	/// Group ID of owner
	gid_t getGid() const;
	
	/// Is the file information correct? Did the query succeed?
	bool isOK() const;
	
	/// Permission flags
	enum perm_test {
		/// test for read permission
		rperm = R_OK,
		/// test for write permission
		wperm = W_OK,
		/// test for execute (search) permission
		xperm = X_OK,
		/// test for existence of file
		eperm = F_OK
	};
	/// Test whether the current user has a given set of permissions
	bool access(int p) const;
	/// Is the file writable for the current user?
	bool writable() const { return access(FileInfo::wperm); }
	/// Is the file readable for the current user?
	bool readable() const { return access(FileInfo::rperm); }
	/// Is the file executable for the current user?
	bool executable() const { return access(FileInfo::xperm); }
	/// Does the file exist?
	bool exist() const { return access(FileInfo::eperm); }
	///
	bool isLink() const;
	///
	bool isRegular() const;
	///
	bool isDir() const;
	///
	bool isChar() const;
	///
	bool isBlock() const;
	///
	bool isFifo() const;
	///
	bool isSocket() const;
	///
	int getError() const;
	///
	enum Err {
		///
		NoErr = -1
	};
private:
	///
	void init();
	///
	void dostat(bool);
	///
	struct stat buf;
	///
	int status;
	///
	int err;
	///
	string fname;
};

#endif

// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#ifndef FILE_INFO_H
#define FILE_INFO_H

#include <ctime>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "LString.h"

/** Use objects of this class to get information about files. */
class FileInfo {
public:
	///
	FileInfo();

	/** Get information about file.
	If link is true, the information is about the link itself, not
	the file that is obtained by tracing the links. */
	FileInfo(string const & path, bool link = false);

	///
	FileInfo(int fildes);

	///
	~FileInfo();

	///
	FileInfo& newFile(string const & path, bool link = false);

	///
        FileInfo& newFile(int fildes);
	
	/// returns a character describing file type (ls -F)
	char const * typeIndicator() const;

	///
	mode_t getMode() const;

	///
	long getBlockSize() const;
	
	/// constructs standard mode string (ls style)
	void modeString(char * szString) const;
	
	/// returns a letter describing a file type (ls style)
	char typeLetter() const;
	
	/// builds 'rwx' string describing file access rights
	void flagRWX(unsigned short i, char * szString) const;
	
	/// updates mode string to match suid/sgid/sticky bits
	void setSticky(char * szString) const;

	///
	time_t getModificationTime() const;

	///
	time_t getAccessTime() const;

	///
	time_t getStatusChangeTime() const;

	///
	off_t getSize() const;

	///
	nlink_t getNumberOfLinks() const;

	///
	uid_t getUid() const;
	///
	gid_t getGid() const;
	///
	bool isOK() const;
	///
	enum perm_test {
		rperm = R_OK, // test for read permission
		wperm = W_OK, // test for write permission
		xperm = X_OK, // test for execute (search) permission
		eperm = F_OK  // test for existence of file
	};
	///
	bool access(int p);
	///
	bool writable() { return access(FileInfo::wperm); }
	///
	bool readable() { return access(FileInfo::rperm); }
	///
	bool executable() { return access(FileInfo::xperm); }
	///
	bool exist() { return access(FileInfo::eperm); }
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
	enum {
		///
		NoErr = -1
	};
private:
	///
	void init();
	///
	void dostat(bool);
	///
	struct stat * buf;
	///
	int status;
	///
	int err;
	///
	string fname;
};

#endif


// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include "FileInfo.h"

#if !S_IRUSR
# if S_IREAD
#  define S_IRUSR S_IREAD
# else
#  define S_IRUSR 00400
# endif
#endif

#if !S_IWUSR
# if S_IWRITE
#  define S_IWUSR S_IWRITE
# else
#  define S_IWUSR 00200
# endif
#endif

#if !S_IXUSR
# if S_IEXEC
#  define S_IXUSR S_IEXEC
# else
#  define S_IXUSR 00100
# endif
#endif

#ifdef STAT_MACROS_BROKEN
#undef S_ISBLK
#undef S_ISCHR
#undef S_ISDIR
#undef S_ISFIFO
#undef S_ISLNK
#undef S_ISMPB
#undef S_ISMPC
#undef S_ISNWK
#undef S_ISREG
#undef S_ISSOCK
#endif 

#if !defined(S_ISBLK) && defined(S_IFBLK)
#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#endif
#if !defined(S_ISCHR) && defined(S_IFCHR)
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif
#if !defined(S_ISDIR) && defined(S_IFDIR)
#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif
#if !defined(S_ISREG) && defined(S_IFREG)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISFIFO) && defined(S_IFIFO)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#endif
#if !defined(S_ISLNK) && defined(S_IFLNK)
#define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
#endif
#if !defined(S_ISSOCK) && defined(S_IFSOCK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#endif
#if !defined(S_ISMPB) && defined(S_IFMPB) /* V7 */
#define S_ISMPB(m) (((m) & S_IFMT) == S_IFMPB)
#define S_ISMPC(m) (((m) & S_IFMT) == S_IFMPC)
#endif
#if !defined(S_ISNWK) && defined(S_IFNWK) /* HP/UX */
#define S_ISNWK(m) (((m) & S_IFMT) == S_IFNWK)
#endif

// Since major is a function on SVR4, we can't use `ifndef major'. 
// might want to put MAJOR_IN_MKDEV for SYSV
#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#define HAVE_MAJOR
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#define HAVE_MAJOR
#endif
#ifdef major		
#define HAVE_MAJOR
#endif

#ifndef HAVE_MAJOR
#define major(dev)  (((dev) >> 8) & 0xff)
#define minor(dev)  ((dev) & 0xff)
#define makedev(maj, min)  (((maj) << 8) | (min))
#endif
#undef HAVE_MAJOR


FileInfo::FileInfo()
{
	init();
}


FileInfo::FileInfo(string const & path, bool link)
	: fname(path)
{
	init();
	dostat(link);
}


FileInfo::FileInfo(int fildes)
{
	init();
	status = fstat(fildes, &buf);
	if (status) err = errno;
}


void FileInfo::init()
{
	status = 0;
	err = NoErr;
}


void FileInfo::dostat(bool link)
{
	if (link) {
		status = lstat(fname.c_str(), &buf);
	} else {
		status = stat(fname.c_str(), &buf);
	}
	if (status) err = errno;
}


FileInfo & FileInfo::newFile(string const & path, bool link)
{
	fname = path;
	
	status = 0;
	err = NoErr;

	dostat(link);

	return *this;
}


FileInfo & FileInfo::newFile(int fildes)
{
	status = 0;
	err = NoErr;
	status = fstat(fildes, &buf);
	if (status) err = errno;
	return *this;
}


char const * FileInfo::typeIndicator() const
{
	if (S_ISDIR(buf.st_mode)) return ("/");
#ifdef S_ISLNK
	if (S_ISLNK(buf.st_mode)) return ("@");
#endif
#ifdef S_ISFIFO
	if (S_ISFIFO(buf.st_mode)) return ("|");
#endif
#ifdef S_ISSOCK
	if (S_ISSOCK(buf.st_mode)) return ("=");
#endif
	if (S_ISREG(buf.st_mode) && (buf.st_mode & (S_IEXEC | S_IXGRP | S_IXOTH)))
		return ("*");
	return "";
}


mode_t FileInfo::getMode() const
{
	return buf.st_mode;
}

long FileInfo::getBlockSize() const
{
#ifndef __EMX__
	return buf.st_blksize; /* Preferred I/O block size */
#else
#warning May be fixed in 0.13 (SMiyata)
	return 512; /* Assume HPFS */
#endif
}

void FileInfo::modeString(char * szString) const
{
	szString[0] = typeLetter();
	flagRWX((buf.st_mode & 0700) << 0, &szString[1]);
	flagRWX((buf.st_mode & 0070) << 3, &szString[4]);
	flagRWX((buf.st_mode & 0007) << 6, &szString[7]);
	setSticky(szString);
	szString[10] = 0;
}


char FileInfo::typeLetter() const
{
#ifdef S_ISBLK
	if (S_ISBLK(buf.st_mode)) return 'b';
#endif
	if (S_ISCHR(buf.st_mode)) return 'c';
	if (S_ISDIR(buf.st_mode)) return 'd';
	if (S_ISREG(buf.st_mode)) return '-';
#ifdef S_ISFIFO
	if (S_ISFIFO(buf.st_mode)) return 'p';
#endif
#ifdef S_ISLNK
	if (S_ISLNK(buf.st_mode)) return 'l';
#endif
#ifdef S_ISSOCK
	if (S_ISSOCK(buf.st_mode)) return 's';
#endif
#ifdef S_ISMPC
	if (S_ISMPC(buf.st_mode)) return 'm';
#endif
#ifdef S_ISNWK
	if (S_ISNWK(buf.st_mode)) return 'n';
#endif
	return '?';
}


void FileInfo::flagRWX(unsigned short i, char * szString) const
{
	szString[0] = (i & S_IRUSR) ? 'r' : '-';
	szString[1] = (i & S_IWUSR) ? 'w' : '-';
	szString[2] = (i & S_IXUSR) ? 'x' : '-';
}


void FileInfo::setSticky(char * szString) const
{
#ifdef S_ISUID
	if (buf.st_mode & S_ISUID) {
		if (szString[3] != 'x') szString[3] = 'S';
		else szString[3] = 's';
	}
#endif
#ifdef S_ISGID
	if (buf.st_mode & S_ISGID) {
		if (szString[6] != 'x') szString[6] = 'S';
		else szString[6] = 's';
	}
#endif
#ifdef S_ISVTX
	if (buf.st_mode & S_ISVTX) {
		if (szString[9] != 'x') szString[9] = 'T';
		else szString[9] = 't';
	}
#endif
}


time_t FileInfo::getModificationTime() const
{
	return buf.st_mtime;
}


time_t FileInfo::getAccessTime() const
{
	return buf.st_atime;
}


time_t  FileInfo::getStatusChangeTime() const
{
	return buf.st_ctime;
}


nlink_t FileInfo::getNumberOfLinks() const
{
	return buf.st_nlink;
}


uid_t FileInfo::getUid() const
{
	return buf.st_uid;
}


gid_t FileInfo::getGid() const
{
	return buf.st_gid;
}


off_t FileInfo::getSize() const
{
	return buf.st_size;
}


int FileInfo::getError() const
{
	return err;
}


bool FileInfo::isOK() const
{
	// DEC cxx 6.0 chokes on this bizarre construct (compiler bug)
	// return (status) ? false : true;
	// So I replaced it with a simpler one (JMarc)
	return status == 0;
}


bool FileInfo::isLink() const
{
	return S_ISLNK(buf.st_mode);
}


bool FileInfo::isRegular() const
{
	return S_ISREG(buf.st_mode);
}


bool FileInfo::isDir() const
{
	return S_ISDIR(buf.st_mode);
}


bool FileInfo::isChar() const
{
	return S_ISCHR(buf.st_mode);
}


bool FileInfo::isBlock() const
{
	return S_ISBLK(buf.st_mode);
}


bool FileInfo::isFifo() const
{
	return S_ISFIFO(buf.st_mode);
}


bool FileInfo::isSocket() const
{
#ifdef S_ISSOCK
	return S_ISSOCK(buf.st_mode);
#else
	return false;
#endif
}


bool FileInfo::access(int p)
{
	// if we don't have a filename we fail
	if (fname.empty()) return false;
	
	if (::access(fname.c_str(), p) == 0)
		return true;
	else {
		// If we were really kind, we would also tell why
		// the file access failed.
		return false;
	}
}

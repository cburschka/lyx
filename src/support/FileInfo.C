/**
 * \file FileInfo.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

//#include <sys/types.h>
//#include <sys/stat.h>

#include <cerrno>
#include "FileInfo.h"
#include "LAssert.h"

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


namespace {

// builds 'rwx' string describing file access rights
void flagRWX(mode_t i, char * str)
{
	str[0] = (i & S_IRUSR) ? 'r' : '-';
	str[1] = (i & S_IWUSR) ? 'w' : '-';
	str[2] = (i & S_IXUSR) ? 'x' : '-';
}

// updates mode string to match suid/sgid/sticky bits
void setSticky(mode_t i, char * str)
{
#ifdef S_ISUID
	if (i & S_ISUID)
		str[3] = (str[3] == 'x') ? 's' : 'S';
#endif
#ifdef S_ISGID
	if (i & S_ISGID)
		str[6] = (str[6] == 'x') ? 's' : 'S';
#endif
#ifdef S_ISVTX
	if (i & S_ISVTX)
		str[9] = (str[9] == 'x') ? 's' : 'S';
#endif
}

// returns a letter describing a file type (ls style)
char typeLetter(mode_t i)
{
#ifdef S_ISBLK
	if (S_ISBLK(i)) return 'b';
#endif
	if (S_ISCHR(i)) return 'c';
	if (S_ISDIR(i)) return 'd';
	if (S_ISREG(i)) return '-';
#ifdef S_ISFIFO
	if (S_ISFIFO(i)) return 'p';
#endif
#ifdef S_ISLNK
	if (S_ISLNK(i)) return 'l';
#endif
#ifdef S_ISSOCK
	if (S_ISSOCK(i)) return 's';
#endif
#ifdef S_ISMPC
	if (S_ISMPC(i)) return 'm';
#endif
#ifdef S_ISNWK
	if (S_ISNWK(i)) return 'n';
#endif
	return '?';
}


} // namespace anon


FileInfo::FileInfo()
{
	init();
}


FileInfo::FileInfo(string const & path, bool link)
	: fname_(path)
{
	init();
	dostat(link);
}


FileInfo::FileInfo(int fildes)
{
	init();
	status_ = fstat(fildes, &buf_);
	if (status_)
		err_ = errno;
}


void FileInfo::init()
{
	status_ = 0;
	err_ = NoErr;
}


void FileInfo::dostat(bool link)
{
	if (link)
		status_ = ::lstat(fname_.c_str(), &buf_);
	else
		status_ = ::stat(fname_.c_str(), &buf_);
	if (status_)
		err_ = errno;
}


FileInfo & FileInfo::newFile(string const & path, bool link)
{
	fname_  = path;
	status_ = 0;
	err_    = NoErr;
	dostat(link);
	return *this;
}


FileInfo & FileInfo::newFile(int fildes)
{
	status_ = 0;
	err_    = NoErr;
	status_ = fstat(fildes, &buf_);
	if (status_)
		err_ = errno;
	return *this;
}


// should not be in FileInfo
char FileInfo::typeIndicator() const
{
	lyx::Assert(isOK());
	if (S_ISDIR(buf_.st_mode))
		return '/';
#ifdef S_ISLNK
	if (S_ISLNK(buf_.st_mode))
		return '@';
#endif
#ifdef S_ISFIFO
	if (S_ISFIFO(buf_.st_mode))
		return '|';
#endif
#ifdef S_ISSOCK
	if (S_ISSOCK(buf_.st_mode))
		return '=';
#endif
	if (S_ISREG(buf_.st_mode) && (buf_.st_mode & (S_IEXEC | S_IXGRP | S_IXOTH)))
		return '*';
	return ' ';
}


mode_t FileInfo::getMode() const
{
	lyx::Assert(isOK());
	return buf_.st_mode;
}


// should not be in FileInfo
string FileInfo::modeString() const
{
	lyx::Assert(isOK());
	char str[11];
	str[0] = typeLetter(buf_.st_mode);
	flagRWX((buf_.st_mode & 0700) << 0, &str[1]);
	flagRWX((buf_.st_mode & 0070) << 3, &str[4]);
	flagRWX((buf_.st_mode & 0007) << 6, &str[7]);
	setSticky(buf_.st_mode, str);
	str[10] = 0;
	return str;
}



time_t FileInfo::getModificationTime() const
{
	lyx::Assert(isOK());
	return buf_.st_mtime;
}


time_t FileInfo::getAccessTime() const
{
	lyx::Assert(isOK());
	return buf_.st_atime;
}


time_t FileInfo::getStatusChangeTime() const
{
	lyx::Assert(isOK());
	return buf_.st_ctime;
}


nlink_t FileInfo::getNumberOfLinks() const
{
	lyx::Assert(isOK());
	return buf_.st_nlink;
}


uid_t FileInfo::getUid() const
{
	lyx::Assert(isOK());
	return buf_.st_uid;
}


gid_t FileInfo::getGid() const
{
	lyx::Assert(isOK());
	return buf_.st_gid;
}


off_t FileInfo::getSize() const
{
	lyx::Assert(isOK());
	return buf_.st_size;
}


int FileInfo::getError() const
{
	return err_;
}


bool FileInfo::isOK() const
{
	return status_ == 0;
}


bool FileInfo::isLink() const
{
	lyx::Assert(isOK());
	return S_ISLNK(buf_.st_mode);
}


bool FileInfo::isRegular() const
{
	lyx::Assert(isOK());
	return S_ISREG(buf_.st_mode);
}


bool FileInfo::isDir() const
{
	lyx::Assert(isOK());
	return S_ISDIR(buf_.st_mode);
}


bool FileInfo::isChar() const
{
	lyx::Assert(isOK());
	return S_ISCHR(buf_.st_mode);
}


bool FileInfo::isBlock() const
{
	lyx::Assert(isOK());
	return S_ISBLK(buf_.st_mode);
}


bool FileInfo::isFifo() const
{
	lyx::Assert(isOK());
	return S_ISFIFO(buf_.st_mode);
}


bool FileInfo::isSocket() const
{
	lyx::Assert(isOK());
#ifdef S_ISSOCK
	return S_ISSOCK(buf_.st_mode);
#else
	return false;
#endif
}


// should not be in FileInfo
bool FileInfo::access(int p) const
{
	// if we don't have a filename we fail
	if (fname_.empty())
		return false;

	// If we were really kind, we would also tell why
	// the file access failed.
	return ::access(fname_.c_str(), p) == 0;
}

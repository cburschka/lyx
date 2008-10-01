/**
 * \file FileName.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/FileName.h"
#include "support/FileNameList.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/filetools.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/qstring_helpers.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/qstring_helpers.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QTemporaryFile>
#include <QTime>

#include <boost/crc.hpp>
#include <boost/scoped_array.hpp>

#include <algorithm>
#include <iterator>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_DIRECT_H
# include <direct.h>
#endif
#ifdef _WIN32
# include <windows.h>
#endif

#include <cerrno>
#include <fcntl.h>

#if defined(HAVE_MKSTEMP) && ! defined(HAVE_DECL_MKSTEMP)
extern "C" int mkstemp(char *);
#endif

#if !defined(HAVE_MKSTEMP) && defined(HAVE_MKTEMP)
# ifdef HAVE_IO_H
#  include <io.h>
# endif
# ifdef HAVE_PROCESS_H
#  include <process.h>
# endif
#endif

// Three implementations of checksum(), depending on having mmap support or not.
#if defined(HAVE_MMAP) && defined(HAVE_MUNMAP)
#define SUM_WITH_MMAP
#include <sys/mman.h>
#endif // SUM_WITH_MMAP

using namespace std;

// OK, this is ugly, but it is the only workaround I found to compile
// with gcc (any version) on a system which uses a non-GNU toolchain.
// The problem is that gcc uses a weak symbol for a particular
// instantiation and that the system linker usually does not
// understand those weak symbols (seen on HP-UX, tru64, AIX and
// others). Thus we force an explicit instanciation of this particular
// template (JMarc)
template struct boost::detail::crc_table_t<32, 0x04C11DB7, true>;

namespace lyx {
namespace support {

/////////////////////////////////////////////////////////////////////
//
// FileName::Private
//
/////////////////////////////////////////////////////////////////////

struct FileName::Private
{
	Private() {}

	Private(string const & abs_filename) : fi(toqstr(abs_filename))
	{
		fi.setCaching(fi.exists() ? true : false);
	}
	///
	inline void refresh() 
	{
// There seems to be a bug in Qt >= 4.2.0, at least, that causes problems with
// QFileInfo::refresh() on *nix. So we recreate the object in that case.
// FIXME: When Trolltech fixes the bug, we will have to replace 0x999999 below
// with the actual working minimum version.
#if defined(_WIN32) || (QT_VERSION >= 0x999999)
		fi.refresh();
#else
		fi = QFileInfo(fi.absoluteFilePath());
#endif
	}

	///
	QFileInfo fi;
};

/////////////////////////////////////////////////////////////////////
//
// FileName
//
/////////////////////////////////////////////////////////////////////


FileName::FileName() : d(new Private)
{
}


FileName::FileName(string const & abs_filename)
	: d(abs_filename.empty() ? new Private : new Private(abs_filename))
{
}


FileName::~FileName()
{
	delete d;
}


FileName::FileName(FileName const & rhs) : d(new Private)
{
	d->fi = rhs.d->fi;
}


FileName::FileName(FileName const & rhs, string const & suffix) : d(new Private)
{
	set(rhs, suffix);
}


FileName & FileName::operator=(FileName const & rhs)
{
	d->fi = rhs.d->fi;
	return *this;
}


bool FileName::empty() const
{
	return d->fi.absoluteFilePath().isEmpty();
}


bool FileName::isAbsolute() const
{
	return d->fi.isAbsolute();
}


string FileName::absFilename() const
{
	return fromqstr(d->fi.absoluteFilePath());
}


void FileName::set(string const & name)
{
	d->fi.setFile(toqstr(name));
}


void FileName::set(FileName const & rhs, string const & suffix)
{
	if (!rhs.d->fi.isDir())
		d->fi.setFile(rhs.d->fi.filePath() + toqstr(suffix));
	else
		d->fi.setFile(QDir(rhs.d->fi.absoluteFilePath()), toqstr(suffix));
}


void FileName::erase()
{
	d->fi = QFileInfo();
}


bool FileName::copyTo(FileName const & name) const
{
	LYXERR(Debug::FILES, "Copying " << name);
	QFile::remove(name.d->fi.absoluteFilePath());
	bool success = QFile::copy(d->fi.absoluteFilePath(), name.d->fi.absoluteFilePath());
	if (!success)
		LYXERR0("FileName::copyTo(): Could not copy file "
			<< *this << " to " << name);
	return success;
}


bool FileName::renameTo(FileName const & name) const
{
	bool success = QFile::rename(d->fi.absoluteFilePath(), name.d->fi.absoluteFilePath());
	if (!success)
		LYXERR0("Could not rename file " << *this << " to " << name);
	return success;
}


bool FileName::moveTo(FileName const & name) const
{
	QFile::remove(name.d->fi.absoluteFilePath());

	bool success = QFile::rename(d->fi.absoluteFilePath(),
		name.d->fi.absoluteFilePath());
	if (!success)
		LYXERR0("Could not move file " << *this << " to " << name);
	return success;
}


bool FileName::changePermission(unsigned long int mode) const
{
#if defined (HAVE_CHMOD) && defined (HAVE_MODE_T)
	if (::chmod(toFilesystemEncoding().c_str(), mode_t(mode)) != 0) {
		LYXERR0("File " << *this << ": cannot change permission to "
			<< mode << ".");
		return false;
	}
#endif
	return true;
}


string FileName::toFilesystemEncoding() const
{
	// FIXME: This doesn't work on Windows for non ascii file names with Qt < 4.4.
	// Provided that Windows package uses Qt4.4, this isn't a problem.
	QByteArray const encoded = QFile::encodeName(d->fi.absoluteFilePath());
	return string(encoded.begin(), encoded.end());
}


FileName FileName::fromFilesystemEncoding(string const & name)
{
	QByteArray const encoded(name.c_str(), name.length());
	return FileName(fromqstr(QFile::decodeName(encoded)));
}


bool FileName::exists() const
{
	return d->fi.exists();
}


bool FileName::isSymLink() const
{
	return d->fi.isSymLink();
}


bool FileName::isFileEmpty() const
{
	return d->fi.size() == 0;
}


bool FileName::isDirectory() const
{
	return d->fi.isDir();
}


bool FileName::isReadOnly() const
{
	return d->fi.isReadable() && !d->fi.isWritable();
}


bool FileName::isReadableDirectory() const
{
	return d->fi.isDir() && d->fi.isReadable();
}


string FileName::onlyFileName() const
{
	return fromqstr(d->fi.fileName());
}


string FileName::onlyFileNameWithoutExt() const
{
       return fromqstr(d->fi.baseName());
}


string FileName::extension() const
{
       return fromqstr(d->fi.suffix());
}


FileName FileName::onlyPath() const
{
	FileName path;
	path.d->fi.setFile(d->fi.path());
	return path;
}


bool FileName::isReadableFile() const
{
	return d->fi.isFile() && d->fi.isReadable();
}


bool FileName::isWritable() const
{
	return d->fi.isWritable();
}


bool FileName::isDirWritable() const
{
	LASSERT(d->fi.isDir(), return false);
	QFileInfo tmp(QDir(d->fi.absoluteFilePath()), "lyxwritetest");
	QTemporaryFile qt_tmp(tmp.absoluteFilePath());
	if (qt_tmp.open()) {
		LYXERR(Debug::FILES, "Directory " << *this << " is writable");
		return true;
	}
	LYXERR(Debug::FILES, "Directory " << *this << " is not writable");
	return false;
}


FileNameList FileName::dirList(string const & ext) const
{
	FileNameList dirlist;
	if (!isDirectory()) {
		LYXERR0("Directory '" << *this << "' does not exist!");
		return dirlist;
	}

	QDir dir = d->fi.absoluteDir();

	if (!ext.empty()) {
		QString filter;
		switch (ext[0]) {
		case '.': filter = "*" + toqstr(ext); break;
		case '*': filter = toqstr(ext); break;
		default: filter = "*." + toqstr(ext);
		}
		dir.setNameFilters(QStringList(filter));
		LYXERR(Debug::FILES, "filtering on extension "
			<< fromqstr(filter) << " is requested.");
	}

	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i != list.size(); ++i) {
		FileName fi(fromqstr(list.at(i).absoluteFilePath()));
		dirlist.push_back(fi);
		LYXERR(Debug::FILES, "found file " << fi);
	}

	return dirlist;
}


static string createTempFile(QString const & mask)
{
	QTemporaryFile qt_tmp(mask);
	if (qt_tmp.open()) {
		string const temp_file = fromqstr(qt_tmp.fileName());
		LYXERR(Debug::FILES, "Temporary file `" << temp_file << "' created.");
		return temp_file;
	}
	LYXERR(Debug::FILES, "Unable to create temporary file with following template: "
		<< qt_tmp.fileTemplate());
	return string();
}


FileName FileName::tempName(FileName const & temp_dir, string const & mask)
{
	QFileInfo tmp_fi(QDir(temp_dir.d->fi.absoluteFilePath()), toqstr(mask));
	LYXERR(Debug::FILES, "Temporary file in " << tmp_fi.absoluteFilePath());
	return FileName(createTempFile(tmp_fi.absoluteFilePath()));
}


FileName FileName::tempName(string const & mask)
{
	return tempName(package().temp_dir(), mask);
}


FileName FileName::getcwd()
{
	return FileName(".");
}


FileName FileName::tempPath()
{
	return FileName(fromqstr(QDir::tempPath()));
}


time_t FileName::lastModified() const
{
	// QFileInfo caches information about the file. So, in case this file has
	// been touched between the object creation and now, we refresh the file
	// information.
	d->refresh();
	return d->fi.lastModified().toTime_t();
}


bool FileName::chdir() const
{
	return QDir::setCurrent(d->fi.absoluteFilePath());
}


unsigned long FileName::checksum() const
{
	unsigned long result = 0;

	if (!exists()) {
		//LYXERR0("File \"" << absFilename() << "\" does not exist!");
		return result;
	}
	// a directory may be passed here so we need to test it. (bug 3622)
	if (isDirectory()) {
		LYXERR0('"' << absFilename() << "\" is a directory!");
		return result;
	}

	// This is used in the debug output at the end of the method.
	static QTime t;
	if (lyxerr.debugging(Debug::FILES))
		t.restart();

#if QT_VERSION >= 0x999999
	// First version of checksum uses Qt4.4 mmap support.
	// FIXME: This code is not ready with Qt4.4.2,
	// see http://bugzilla.lyx.org/show_bug.cgi?id=5293
	// FIXME: should we check if the MapExtension extension is supported?
	// see QAbstractFileEngine::supportsExtension() and 
	// QAbstractFileEngine::MapExtension)
	QFile qf(fi.filePath());
	if (!qf.open(QIODevice::ReadOnly))
		return result;
	qint64 size = fi.size();
	uchar * ubeg = qf.map(0, size);
	uchar * uend = ubeg + size;
	boost::crc_32_type ucrc;
	ucrc.process_block(ubeg, uend);
	qf.unmap(ubeg);
	qf.close();
	result = ucrc.checksum();

#else // QT_VERSION

	string const encoded = toFilesystemEncoding();
	char const * file = encoded.c_str();

 #ifdef SUM_WITH_MMAP
	//LYXERR(Debug::FILES, "using mmap (lightning fast)");

	int fd = open(file, O_RDONLY);
	if (!fd)
		return result;

	struct stat info;
	fstat(fd, &info);

	void * mm = mmap(0, info.st_size, PROT_READ,
			 MAP_PRIVATE, fd, 0);
	// Some platforms have the wrong type for MAP_FAILED (compaq cxx).
	if (mm == reinterpret_cast<void*>(MAP_FAILED)) {
		close(fd);
		return result;
	}

	char * beg = static_cast<char*>(mm);
	char * end = beg + info.st_size;

	boost::crc_32_type crc;
	crc.process_block(beg, end);
	result = crc.checksum();

	munmap(mm, info.st_size);
	close(fd);

 #else // no SUM_WITH_MMAP

	//LYXERR(Debug::FILES, "lyx::sum() using istreambuf_iterator (fast)");
	ifstream ifs(file, ios_base::in | ios_base::binary);
	if (!ifs)
		return result;

	istreambuf_iterator<char> beg(ifs);
	istreambuf_iterator<char> end;
	boost::crc_32_type crc;
	crc = for_each(beg, end, crc);
	result = crc.checksum();

 #endif // SUM_WITH_MMAP
#endif // QT_VERSION

	LYXERR(Debug::FILES, "Checksumming \"" << absFilename() << "\" "
		<< result << " lasted " << t.elapsed() << " ms.");
	return result;
}


bool FileName::removeFile() const
{
	bool const success = QFile::remove(d->fi.absoluteFilePath());
	if (!success && exists())
		LYXERR0("Could not delete file " << *this);
	return success;
}


static bool rmdir(QFileInfo const & fi)
{
	QDir dir(fi.absoluteFilePath());
	QFileInfoList list = dir.entryInfoList();
	bool success = true;
	for (int i = 0; i != list.size(); ++i) {
		if (list.at(i).fileName() == ".")
			continue;
		if (list.at(i).fileName() == "..")
			continue;
		bool removed;
		if (list.at(i).isDir()) {
			LYXERR(Debug::FILES, "Removing dir " 
				<< fromqstr(list.at(i).absoluteFilePath()));
			removed = rmdir(list.at(i));
		}
		else {
			LYXERR(Debug::FILES, "Removing file " 
				<< fromqstr(list.at(i).absoluteFilePath()));
			removed = dir.remove(list.at(i).fileName());
		}
		if (!removed) {
			success = false;
			LYXERR0("Could not delete "
				<< fromqstr(list.at(i).absoluteFilePath()));
		}
	} 
	QDir parent = fi.absolutePath();
	success &= parent.rmdir(fi.fileName());
	return success;
}


bool FileName::destroyDirectory() const
{
	bool const success = rmdir(d->fi);
	if (!success)
		LYXERR0("Could not delete " << *this);

	return success;
}


// Only used in non Win32 platforms
static int mymkdir(char const * pathname, unsigned long int mode)
{
	// FIXME: why don't we have mode_t in lyx::mkdir prototype ??
#if HAVE_MKDIR
# if MKDIR_TAKES_ONE_ARG
	// MinGW32
	return ::mkdir(pathname);
	// FIXME: "Permissions of created directories are ignored on this system."
# else
	// POSIX
	return ::mkdir(pathname, mode_t(mode));
# endif
#elif defined(_WIN32)
	// plain Windows 32
	return CreateDirectory(pathname, 0) != 0 ? 0 : -1;
	// FIXME: "Permissions of created directories are ignored on this system."
#elif HAVE__MKDIR
	return ::_mkdir(pathname);
	// FIXME: "Permissions of created directories are ignored on this system."
#else
#   error "Don't know how to create a directory on this system."
#endif

}


bool FileName::createDirectory(int permission) const
{
	LASSERT(!empty(), return false);
#ifdef Q_OS_WIN32
	// FIXME: "Permissions of created directories are ignored on this system."
	return createPath();
#else
	return mymkdir(toFilesystemEncoding().c_str(), permission) == 0;
#endif
}


bool FileName::createPath() const
{
	LASSERT(!empty(), /**/);
	LYXERR(Debug::FILES, "creating path '" << *this << "'.");
	if (isDirectory())
		return false;

	QDir dir;
	bool success = dir.mkpath(d->fi.absoluteFilePath());
	if (!success)
		LYXERR0("Cannot create path '" << *this << "'!");
	return success;
}


docstring const FileName::absoluteFilePath() const
{
	return qstring_to_ucs4(d->fi.absoluteFilePath());
}


docstring FileName::displayName(int threshold) const
{
	return makeDisplayPath(absFilename(), threshold);
}


docstring FileName::fileContents(string const & encoding) const
{
	if (!isReadableFile()) {
		LYXERR0("File '" << *this << "' is not redable!");
		return docstring();
	}

	QFile file(d->fi.absoluteFilePath());
	if (!file.open(QIODevice::ReadOnly)) {
		LYXERR0("File '" << *this
			<< "' could not be opened in read only mode!");
		return docstring();
	}
	QByteArray contents = file.readAll();
	file.close();

	if (contents.isEmpty()) {
		LYXERR(Debug::FILES, "File '" << *this
			<< "' is either empty or some error happened while reading it.");
		return docstring();
	}

	QString s;
	if (encoding.empty() || encoding == "UTF-8")
		s = QString::fromUtf8(contents.data());
	else if (encoding == "ascii")
		s = QString::fromAscii(contents.data());
	else if (encoding == "local8bit")
		s = QString::fromLocal8Bit(contents.data());
	else if (encoding == "latin1")
		s = QString::fromLatin1(contents.data());

	return qstring_to_ucs4(s);
}


void FileName::changeExtension(string const & extension)
{
	// FIXME: use Qt native methods...
	string const oldname = absFilename();
	string::size_type const last_slash = oldname.rfind('/');
	string::size_type last_dot = oldname.rfind('.');
	if (last_dot < last_slash && last_slash != string::npos)
		last_dot = string::npos;

	string ext;
	// Make sure the extension starts with a dot
	if (!extension.empty() && extension[0] != '.')
		ext= '.' + extension;
	else
		ext = extension;

	set(oldname.substr(0, last_dot) + ext);
}


string FileName::guessFormatFromContents() const
{
	// the different filetypes and what they contain in one of the first lines
	// (dots are any characters).		(Herbert 20020131)
	// AGR	Grace...
	// BMP	BM...
	// EPS	%!PS-Adobe-3.0 EPSF...
	// FIG	#FIG...
	// FITS ...BITPIX...
	// GIF	GIF...
	// JPG	JFIF
	// PDF	%PDF-...
	// PNG	.PNG...
	// PBM	P1... or P4	(B/W)
	// PGM	P2... or P5	(Grayscale)
	// PPM	P3... or P6	(color)
	// PS	%!PS-Adobe-2.0 or 1.0,  no "EPSF"!
	// SGI	\001\332...	(decimal 474)
	// TGIF	%TGIF...
	// TIFF	II... or MM...
	// XBM	..._bits[]...
	// XPM	/* XPM */    sometimes missing (f.ex. tgif-export)
	//      ...static char *...
	// XWD	\000\000\000\151	(0x00006900) decimal 105
	//
	// GZIP	\037\213	http://www.ietf.org/rfc/rfc1952.txt
	// ZIP	PK...			http://www.halyava.ru/document/ind_arch.htm
	// Z	\037\235		UNIX compress

	// paranoia check
	if (empty() || !isReadableFile())
		return string();

	ifstream ifs(toFilesystemEncoding().c_str());
	if (!ifs)
		// Couldn't open file...
		return string();

	// gnuzip
	static string const gzipStamp = "\037\213";

	// PKZIP
	static string const zipStamp = "PK";

	// compress
	static string const compressStamp = "\037\235";

	// Maximum strings to read
	int const max_count = 50;
	int count = 0;

	string str;
	string format;
	bool firstLine = true;
	while ((count++ < max_count) && format.empty()) {
		if (ifs.eof()) {
			LYXERR(Debug::GRAPHICS, "filetools(getFormatFromContents)\n"
				<< "\tFile type not recognised before EOF!");
			break;
		}

		getline(ifs, str);
		string const stamp = str.substr(0, 2);
		if (firstLine && str.size() >= 2) {
			// at first we check for a zipped file, because this
			// information is saved in the first bytes of the file!
			// also some graphic formats which save the information
			// in the first line, too.
			if (prefixIs(str, gzipStamp)) {
				format =  "gzip";

			} else if (stamp == zipStamp) {
				format =  "zip";

			} else if (stamp == compressStamp) {
				format =  "compress";

			// the graphics part
			} else if (stamp == "BM") {
				format =  "bmp";

			} else if (stamp == "\001\332") {
				format =  "sgi";

			// PBM family
			// Don't need to use str.at(0), str.at(1) because
			// we already know that str.size() >= 2
			} else if (str[0] == 'P') {
				switch (str[1]) {
				case '1':
				case '4':
					format =  "pbm";
				    break;
				case '2':
				case '5':
					format =  "pgm";
				    break;
				case '3':
				case '6':
					format =  "ppm";
				}
				break;

			} else if ((stamp == "II") || (stamp == "MM")) {
				format =  "tiff";

			} else if (prefixIs(str,"%TGIF")) {
				format =  "tgif";

			} else if (prefixIs(str,"#FIG")) {
				format =  "fig";

			} else if (prefixIs(str,"GIF")) {
				format =  "gif";

			} else if (str.size() > 3) {
				int const c = ((str[0] << 24) & (str[1] << 16) &
					       (str[2] << 8)  & str[3]);
				if (c == 105) {
					format =  "xwd";
				}
			}

			firstLine = false;
		}

		if (!format.empty())
		    break;
		else if (contains(str,"EPSF"))
			// dummy, if we have wrong file description like
			// %!PS-Adobe-2.0EPSF"
			format = "eps";

		else if (contains(str, "Grace"))
			format = "agr";

		else if (contains(str, "JFIF"))
			format = "jpg";

		else if (contains(str, "%PDF"))
			format = "pdf";

		else if (contains(str, "PNG"))
			format = "png";

		else if (contains(str, "%!PS-Adobe")) {
			// eps or ps
			ifs >> str;
			if (contains(str,"EPSF"))
				format = "eps";
			else
			    format = "ps";
		}

		else if (contains(str, "_bits[]"))
			format = "xbm";

		else if (contains(str, "XPM") || contains(str, "static char *"))
			format = "xpm";

		else if (contains(str, "BITPIX"))
			format = "fits";
	}

	if (!format.empty()) {
		LYXERR(Debug::GRAPHICS, "Recognised Fileformat: " << format);
		return format;
	}

	LYXERR(Debug::GRAPHICS, "filetools(getFormatFromContents)\n"
		<< "\tCouldn't find a known format!");
	return string();
}


bool FileName::isZippedFile() const
{
	string const type = guessFormatFromContents();
	return contains("gzip zip compress", type) && !type.empty();
}


docstring const FileName::relPath(string const & path) const
{
	// FIXME UNICODE
	return makeRelPath(absoluteFilePath(), from_utf8(path));
}


bool operator==(FileName const & lhs, FileName const & rhs)
{
	// FIXME: We need to solve this warning from Qt documentation:
	// * Long and short file names that refer to the same file on Windows are
	//   treated as if they referred to different files.
	// This is supposed to be fixed for Qt5.

	if (lhs.empty())
		// QFileInfo::operator==() returns false if the two QFileInfo are empty.
		return rhs.empty();

	if (rhs.empty())
		// Avoid unnecessary checks below.
		return false;

	lhs.d->refresh();
	rhs.d->refresh();
	
	if (!lhs.d->fi.isSymLink() && !rhs.d->fi.isSymLink())
		return lhs.d->fi == rhs.d->fi;

	// FIXME: When/if QFileInfo support symlink comparison, remove this code.
	QFileInfo fi1(lhs.d->fi);
	if (fi1.isSymLink())
		fi1 = QFileInfo(fi1.symLinkTarget());
	QFileInfo fi2(rhs.d->fi);
	if (fi2.isSymLink())
		fi2 = QFileInfo(fi2.symLinkTarget());
	return fi1 == fi2;
}


bool operator!=(FileName const & lhs, FileName const & rhs)
{
	return !(operator==(lhs, rhs));
}


bool operator<(FileName const & lhs, FileName const & rhs)
{
	return lhs.absFilename() < rhs.absFilename();
}


bool operator>(FileName const & lhs, FileName const & rhs)
{
	return lhs.absFilename() > rhs.absFilename();
}


ostream & operator<<(ostream & os, FileName const & filename)
{
	return os << filename.absFilename();
}


/////////////////////////////////////////////////////////////////////
//
// DocFileName
//
/////////////////////////////////////////////////////////////////////


DocFileName::DocFileName()
	: save_abs_path_(true)
{}


DocFileName::DocFileName(string const & abs_filename, bool save_abs)
	: FileName(abs_filename), save_abs_path_(save_abs), zipped_valid_(false)
{}


DocFileName::DocFileName(FileName const & abs_filename, bool save_abs)
	: FileName(abs_filename), save_abs_path_(save_abs), zipped_valid_(false)
{}


void DocFileName::set(string const & name, string const & buffer_path)
{
	FileName::set(name);
	bool const nameIsAbsolute = isAbsolute();
	save_abs_path_ = nameIsAbsolute;
	if (!nameIsAbsolute)
		FileName::set(makeAbsPath(name, buffer_path).absFilename());
	zipped_valid_ = false;
}


void DocFileName::erase()
{
	FileName::erase();
	zipped_valid_ = false;
}


string DocFileName::relFilename(string const & path) const
{
	// FIXME UNICODE
	return to_utf8(relPath(path));
}


string DocFileName::outputFilename(string const & path) const
{
	return save_abs_path_ ? absFilename() : relFilename(path);
}


string DocFileName::mangledFilename(string const & dir) const
{
	// We need to make sure that every DocFileName instance for a given
	// filename returns the same mangled name.
	typedef map<string, string> MangledMap;
	static MangledMap mangledNames;
	MangledMap::const_iterator const it = mangledNames.find(absFilename());
	if (it != mangledNames.end())
		return (*it).second;

	string const name = absFilename();
	// Now the real work
	string mname = os::internal_path(name);
	// Remove the extension.
	mname = support::changeExtension(name, string());
	// The mangled name must be a valid LaTeX name.
	// The list of characters to keep is probably over-restrictive,
	// but it is not really a problem.
	// Apart from non-ASCII characters, at least the following characters
	// are forbidden: '/', '.', ' ', and ':'.
	// On windows it is not possible to create files with '<', '>' or '?'
	// in the name.
	static string const keep = "abcdefghijklmnopqrstuvwxyz"
				   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				   "+,-0123456789;=";
	string::size_type pos = 0;
	while ((pos = mname.find_first_not_of(keep, pos)) != string::npos)
		mname[pos++] = '_';
	// Add the extension back on
	mname = support::changeExtension(mname, getExtension(name));

	// Prepend a counter to the filename. This is necessary to make
	// the mangled name unique.
	static int counter = 0;
	ostringstream s;
	s << counter++ << mname;
	mname = s.str();

	// MiKTeX's YAP (version 2.4.1803) crashes if the file name
	// is longer than about 160 characters. MiKTeX's pdflatex
	// is even pickier. A maximum length of 100 has been proven to work.
	// If dir.size() > max length, all bets are off for YAP. We truncate
	// the filename nevertheless, keeping a minimum of 10 chars.

	string::size_type max_length = max(100 - ((int)dir.size() + 1), 10);

	// If the mangled file name is too long, hack it to fit.
	// We know we're guaranteed to have a unique file name because
	// of the counter.
	if (mname.size() > max_length) {
		int const half = (int(max_length) / 2) - 2;
		if (half > 0) {
			mname = mname.substr(0, half) + "___" +
				mname.substr(mname.size() - half);
		}
	}

	mangledNames[absFilename()] = mname;
	return mname;
}


bool DocFileName::isZipped() const
{
	if (!zipped_valid_) {
		zipped_ = isZippedFile();
		zipped_valid_ = true;
	}
	return zipped_;
}


string DocFileName::unzippedFilename() const
{
	return unzippedFileName(absFilename());
}


bool operator==(DocFileName const & lhs, DocFileName const & rhs)
{
	return static_cast<FileName const &>(lhs)
		== static_cast<FileName const &>(rhs)
		&& lhs.saveAbsPath() == rhs.saveAbsPath();
}


bool operator!=(DocFileName const & lhs, DocFileName const & rhs)
{
	return !(lhs == rhs);
}

} // namespace support
} // namespace lyx

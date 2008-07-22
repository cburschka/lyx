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
#include "support/lstrings.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/qstring_helpers.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QTime>

#include "support/lassert.h"
#include <boost/scoped_array.hpp>

#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>

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


#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

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

using namespace std;

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
	LYXERR(Debug::FILES, "isDirWriteable: " << *this);

	FileName const tmpfl = FileName::tempName(absFilename() + "/lyxwritetest");

	if (tmpfl.empty())
		return false;

	tmpfl.removeFile();
	return true;
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


static int make_tempfile(char * templ)
{
#if defined(HAVE_MKSTEMP)
	return ::mkstemp(templ);
#elif defined(HAVE_MKTEMP)
	// This probably just barely works...
	::mktemp(templ);
# if defined (HAVE_OPEN)
# if (!defined S_IRUSR)
#   define S_IRUSR S_IREAD
#   define S_IWUSR S_IWRITE
# endif
	return ::open(templ, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
# elif defined (HAVE__OPEN)
	return ::_open(templ,
		       _O_RDWR | _O_CREAT | _O_EXCL,
		       _S_IREAD | _S_IWRITE);
# else
#  error No open() function.
# endif
#else
#error FIX FIX FIX
#endif
}


FileName FileName::tempName(string const & mask)
{
	FileName tmp_name(mask);
	string tmpfl;
	if (tmp_name.d->fi.isAbsolute())
		tmpfl = mask;
	else
		tmpfl = package().temp_dir().absFilename() + "/" + mask;

#if defined (HAVE_GETPID)
	tmpfl += convert<string>(getpid());
#elif defined (HAVE__GETPID)
	tmpfl += convert<string>(_getpid());
#else
# error No getpid() function
#endif
	tmpfl += "XXXXXX";

	// The supposedly safe mkstemp version
	// FIXME: why not using std::string directly?
	boost::scoped_array<char> tmpl(new char[tmpfl.length() + 1]); // + 1 for '\0'
	tmpfl.copy(tmpl.get(), string::npos);
	tmpl[tmpfl.length()] = '\0'; // terminator

	int const tmpf = make_tempfile(tmpl.get());
	if (tmpf != -1) {
		string const t(to_utf8(from_filesystem8bit(tmpl.get())));
#if defined (HAVE_CLOSE)
		::close(tmpf);
#elif defined (HAVE__CLOSE)
		::_close(tmpf);
#else
# error No x() function.
#endif
		LYXERR(Debug::FILES, "Temporary file `" << t << "' created.");
		return FileName(t);
	}
	LYXERR(Debug::FILES, "LyX Error: Unable to create temporary file.");
	return FileName();
}


FileName FileName::getcwd()
{
	return FileName(".");
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


extern unsigned long sum(char const * file);

unsigned long FileName::checksum() const
{
	if (!exists()) {
		//LYXERR0("File \"" << absFilename() << "\" does not exist!");
		return 0;
	}
	// a directory may be passed here so we need to test it. (bug 3622)
	if (isDirectory()) {
		LYXERR0('"' << absFilename() << "\" is a directory!");
		return 0;
	}
	if (!lyxerr.debugging(Debug::FILES))
		return sum(absFilename().c_str());

	QTime t;
	t.start();
	unsigned long r = sum(absFilename().c_str());
	lyxerr << "Checksumming \"" << absFilename() << "\" lasted "
		<< t.elapsed() << " ms." << endl;
	return r;
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
	LASSERT(!empty(), /**/);
	return mymkdir(toFilesystemEncoding().c_str(), permission) == 0;
}


bool FileName::createPath() const
{
	LASSERT(!empty(), /**/);
	if (isDirectory())
		return true;

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
	return lhs.absFilename() == rhs.absFilename();
}


bool operator!=(FileName const & lhs, FileName const & rhs)
{
	return lhs.absFilename() != rhs.absFilename();
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
	return lhs.absFilename() == rhs.absFilename()
		&& lhs.saveAbsPath() == rhs.saveAbsPath();
}


bool operator!=(DocFileName const & lhs, DocFileName const & rhs)
{
	return !(lhs == rhs);
}

} // namespace support
} // namespace lyx

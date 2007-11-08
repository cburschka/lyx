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
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/qstring_helpers.h"

#include "debug.h"
#include "lyxlib.h"

#include <QFile>
#include <QFileInfo>

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <map>
#include <sstream>
#include <fstream>
#include <algorithm>


using std::map;
using std::string;
using std::ifstream;
using std::ostringstream;
using std::endl;

namespace fs = boost::filesystem;

// FIXME: merge this
//
#include <boost/filesystem/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/throw_exception.hpp>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#include <cerrno>
#include <fcntl.h>


// BOOST_POSIX or BOOST_WINDOWS specify which API to use.
# if !defined( BOOST_WINDOWS ) && !defined( BOOST_POSIX )
#   if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__CYGWIN__)
#     define BOOST_WINDOWS
#   else
#     define BOOST_POSIX
#   endif
# endif

#if defined (BOOST_WINDOWS)
# define WIN32_LEAN_AND_MEAN
# include <windows.h>
#endif


static bool copy_file(std::string const & source, std::string const & target, bool noclobber)
{

#ifdef BOOST_POSIX
	int const infile = ::open(source.c_str(), O_RDONLY);
	if (infile == -1)
		return false;

	struct stat source_stat;
	int const ret = ::fstat(infile, &source_stat);
	if (ret == -1) {
		//int err = errno;
		::close(infile);
	}

	int const flags = O_WRONLY | O_CREAT | (noclobber ? O_EXCL : O_TRUNC);

	int const outfile = ::open(target.c_str(), flags, source_stat.st_mode);
	if (outfile == -1) {
		//int err = errno;
		::close(infile);
		return false;
	}

	std::size_t const buf_sz = 32768;
	char buf[buf_sz];
	ssize_t in = -1;
	ssize_t out = -1;

	while (true) {
		in = ::read(infile, buf, buf_sz);
		if (in == -1) {
			break;
		} else if (in == 0) {
			break;
		} else {
			out = ::write(outfile, buf, in);
			if (out == -1) {
				break;
			}
		}
	}

	//int err = errno;

	::close(infile);
	::close(outfile);

	if (in == -1 || out == -1)
		return false;
#endif

#ifdef BOOST_WINDOWS
	if (::CopyFile(source.c_str(), target.c_str(), noclobber) == 0) {
		// CopyFile is probably not setting errno so this is most
		// likely wrong.
		return false;
	}
#endif
	return true;
}


namespace lyx {
namespace support {


/////////////////////////////////////////////////////////////////////
//
// FileName
//
/////////////////////////////////////////////////////////////////////


FileName::FileName(string const & abs_filename)
	: name_(abs_filename)
{
	BOOST_ASSERT(empty() || absolutePath(name_));
#if defined(_WIN32)
	BOOST_ASSERT(!contains(name_, '\\'));
#endif
}


void FileName::set(string const & name)
{
	name_ = name;
	BOOST_ASSERT(absolutePath(name_));
#if defined(_WIN32)
	BOOST_ASSERT(!contains(name_, '\\'));
#endif
}


void FileName::erase()
{
	name_.erase();
}


bool FileName::copyTo(FileName const & name, bool noclobber) const
{
	try {
		copy_file(toFilesystemEncoding(), name.toFilesystemEncoding(), noclobber);
		return true;
	}
	catch (...) {
	}
	return false;
}


string FileName::toFilesystemEncoding() const
{
	QByteArray const encoded = QFile::encodeName(toqstr(name_));
	return string(encoded.begin(), encoded.end());
}


FileName FileName::fromFilesystemEncoding(string const & name)
{
	QByteArray const encoded(name.c_str(), name.length());
	return FileName(fromqstr(QFile::decodeName(encoded)));
}


bool FileName::exists() const
{
	return QFileInfo(toqstr(name_)).exists();
}


bool FileName::isDirectory() const
{
	return QFileInfo(toqstr(name_)).isDir();
}


bool FileName::isReadOnly() const
{
	QFileInfo const fi(toqstr(name_));
	return fi.isReadable() && !fi.isWritable();
}


bool FileName::isReadable() const
{
	QFileInfo const fi(toqstr(name_));
	return fi.isReadable();
}


std::string FileName::onlyFileName() const
{
	return support::onlyFilename(absFilename());
}


std::string FileName::onlyPath() const
{
	return support::onlyPath(absFilename());
}


bool FileName::isFileReadable() const
{
	QFileInfo const fi(toqstr(name_));
	return fi.isFile() && fi.isReadable();
}


bool FileName::isWritable() const
{
	QFileInfo const fi(toqstr(name_));
	return fi.isWritable();
}


bool FileName::isDirWritable() const
{
	LYXERR(Debug::FILES) << "isDirWriteable: " << *this << std::endl;

	FileName const tmpfl(tempName(*this, "lyxwritetest"));

	if (tmpfl.empty())
		return false;

	unlink(tmpfl);
	return true;
}


FileName FileName::tempName(FileName const & dir, std::string const & mask)
{
	return support::tempName(dir, mask);
}


std::time_t FileName::lastModified() const
{
	return fs::last_write_time(toFilesystemEncoding());
}


bool FileName::destroyDirectory() const
{
	try {
		return fs::remove_all(toFilesystemEncoding()) > 0;
	} catch (fs::filesystem_error const & fe){
		lyxerr << "Could not delete " << *this << ". (" << fe.what() << ")"
			<< std::endl;
		return false;
	}
}


bool FileName::createDirectory(int permission) const
{
	BOOST_ASSERT(!empty());
	return mkdir(*this, permission) == 0;
}


docstring FileName::displayName(int threshold) const
{
	return makeDisplayPath(absFilename(), threshold);
}


string FileName::fileContents() const
{
	if (exists()) {
		string const encodedname = toFilesystemEncoding();
		ifstream ifs(encodedname.c_str());
		ostringstream ofs;
		if (ifs && ofs) {
			ofs << ifs.rdbuf();
			ifs.close();
			return ofs.str();
		}
	}
	lyxerr << "LyX was not able to read file '" << *this << '\'' << std::endl;
	return string();
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

	if (empty() || !isReadable())
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
			LYXERR(Debug::GRAPHICS)
				<< "filetools(getFormatFromContents)\n"
				<< "\tFile type not recognised before EOF!"
				<< endl;
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
		LYXERR(Debug::GRAPHICS)
			<< "Recognised Fileformat: " << format << endl;
		return format;
	}

	LYXERR(Debug::GRAPHICS)
		<< "filetools(getFormatFromContents)\n"
		<< "\tCouldn't find a known format!\n";
	return string();
}


bool FileName::isZippedFile() const
{
	string const type = guessFormatFromContents();
	return contains("gzip zip compress", type) && !type.empty();
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


std::ostream & operator<<(std::ostream & os, FileName const & filename)
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
	save_abs_path_ = absolutePath(name);
	name_ = save_abs_path_ ? name : makeAbsPath(name, buffer_path).absFilename();
	zipped_valid_ = false;
}


void DocFileName::erase()
{
	name_.erase();
	zipped_valid_ = false;
}


string const DocFileName::relFilename(string const & path) const
{
	// FIXME UNICODE
	return to_utf8(makeRelPath(from_utf8(name_), from_utf8(path)));
}


string const DocFileName::outputFilename(string const & path) const
{
	// FIXME UNICODE
	return save_abs_path_ ? name_ : to_utf8(makeRelPath(from_utf8(name_), from_utf8(path)));
}


string const DocFileName::mangledFilename(std::string const & dir) const
{
	// We need to make sure that every DocFileName instance for a given
	// filename returns the same mangled name.
	typedef map<string, string> MangledMap;
	static MangledMap mangledNames;
	MangledMap::const_iterator const it = mangledNames.find(name_);
	if (it != mangledNames.end())
		return (*it).second;

	// Now the real work
	string mname = os::internal_path(name_);
	// Remove the extension.
	mname = changeExtension(name_, string());
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
	mname = changeExtension(mname, getExtension(name_));

	// Prepend a counter to the filename. This is necessary to make
	// the mangled name unique.
	static int counter = 0;
	std::ostringstream s;
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

	mangledNames[name_] = mname;
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


string const DocFileName::unzippedFilename() const
{
	return unzippedFileName(name_);
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




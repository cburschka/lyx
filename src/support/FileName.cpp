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

#include "support/debug.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/os.h"
#include "support/qstring_helpers.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QList>

#include <boost/assert.hpp>

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
#include <cerrno>
#include <fcntl.h>


using std::map;
using std::string;
using std::ifstream;
using std::ostringstream;
using std::endl;

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
	{}
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


string FileName::absFilename() const
{
	return fromqstr(d->fi.absoluteFilePath());
}


void FileName::set(string const & name)
{
	d->fi.setFile(toqstr(name));
	BOOST_ASSERT(d->fi.isAbsolute());
}


void FileName::erase()
{
	d->fi = QFileInfo();
}


bool FileName::copyTo(FileName const & name, bool overwrite) const
{
	if (overwrite)
		QFile::remove(name.d->fi.absoluteFilePath());
	bool success = QFile::copy(d->fi.absoluteFilePath(), name.d->fi.absoluteFilePath());
	if (!success)
		lyxerr << "FileName::copyTo(): Could not copy file "
			<< *this << " to " << name << endl;
	return success;
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


std::string FileName::onlyFileName() const
{
	return support::onlyFilename(absFilename());
}


FileName FileName::onlyPath() const
{
	return FileName(support::onlyPath(absFilename()));
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

	FileName const tmpfl(tempName(*this, "lyxwritetest"));

	if (tmpfl.empty())
		return false;

	tmpfl.removeFile();
	return true;
}


FileName FileName::tempName(FileName const & dir, std::string const & mask)
{
	return support::tempName(dir, mask);
}


std::time_t FileName::lastModified() const
{
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
		LYXERR0("File \"" << absFilename() << "\" does not exist!");
		return 0;
	}
	// a directory may be passed here so we need to test it. (bug 3622)
	if (isDirectory()) {
		LYXERR0('\\' << absFilename() << "\" is a directory!");
		return 0;
	}
	return sum(absFilename().c_str());
}


bool FileName::removeFile() const
{
	bool const success = QFile::remove(d->fi.absoluteFilePath());
	if (!success && exists())
		lyxerr << "FileName::removeFile(): Could not delete file "
			<< *this << "." << endl;
	return success;
}


static bool rmdir(QFileInfo const & fi)
{
	QDir dir(fi.absoluteFilePath());
	QFileInfoList list = dir.entryInfoList();
	bool global_success = true;
	for (int i = 0; i != list.size(); ++i) {
		if (list.at(i).fileName() == ".")
			continue;
		if (list.at(i).fileName() == "..")
			continue;
		bool success;
		if (list.at(i).isDir()) {
			LYXERR(Debug::FILES, "Erasing dir " 
				<< fromqstr(list.at(i).absoluteFilePath()));
			success = rmdir(list.at(i));
		}
		else {
			LYXERR(Debug::FILES, "Erasing file " 
				<< fromqstr(list.at(i).absoluteFilePath()));
			success = dir.remove(list.at(i).fileName());
		}
		if (!success) {
			global_success = false;
			lyxerr << "Could not delete "
				<< fromqstr(list.at(i).absoluteFilePath()) << "." << endl;
		}
	} 
	QDir parent = fi.absolutePath();
	global_success |= parent.rmdir(fi.fileName());
	return global_success;
}


bool FileName::destroyDirectory() const
{
	bool const success = rmdir(d->fi);
	if (!success)
		lyxerr << "Could not delete " << *this << "." << endl;

	return success;
}


bool FileName::createDirectory(int permission) const
{
	BOOST_ASSERT(!empty());
	return mkdir(*this, permission) == 0;
}


std::vector<FileName> dirList(FileName const & dirname, std::string const & ext)
{
	std::vector<FileName> dirlist;
	if (!dirname.isDirectory()) {
		LYXERR0("Directory '" << dirname << "' does not exist!");
		return dirlist;
	}

	QDir dir(dirname.d->fi.absoluteFilePath());

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
		FileName fi;
		fi.d->fi = list.at(i);
		dirlist.push_back(fi);
		LYXERR(Debug::FILES, "found file " << fi);
	}

	return dirlist;
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


void FileName::changeExtension(std::string const & extension)
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
	FileName::set(save_abs_path_ ? name : makeAbsPath(name, buffer_path).absFilename());
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
	return to_utf8(makeRelPath(qstring_to_ucs4(d->fi.absoluteFilePath()), from_utf8(path)));
}


string DocFileName::outputFilename(string const & path) const
{
	return save_abs_path_ ? absFilename() : relFilename(path);
}


string DocFileName::mangledFilename(std::string const & dir) const
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
	std::ostringstream s;
	s << counter++ << mname;
	mname = s.str();

	// MiKTeX's YAP (version 2.4.1803) crashes if the file name
	// is longer than about 160 characters. MiKTeX's pdflatex
	// is even pickier. A maximum length of 100 has been proven to work.
	// If dir.size() > max length, all bets are off for YAP. We truncate
	// the filename nevertheless, keeping a minimum of 10 chars.

	string::size_type max_length = std::max(100 - ((int)dir.size() + 1), 10);

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

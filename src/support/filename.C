/**
 * \file filename.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/filename.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/os.h"
#include "support/qstring_helpers.h"

#include <QFile>

#include <boost/assert.hpp>

#include <map>
#include <sstream>
#include <algorithm>


using std::map;
using std::string;


namespace lyx {
namespace support {


FileName::FileName()
{}


FileName::~FileName()
{}


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


string const FileName::toFilesystemEncoding() const
{
	QByteArray const encoded = QFile::encodeName(toqstr(name_));
	return string(encoded.begin(), encoded.end());
}


FileName const FileName::fromFilesystemEncoding(string const & name)
{
	QByteArray const encoded(name.c_str(), name.length());
	return FileName(fromqstr(QFile::decodeName(encoded)));
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
	return makeRelPath(name_, path);
}


string const DocFileName::outputFilename(string const & path) const
{
	return save_abs_path_ ? name_ : makeRelPath(name_, path);
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
	// Replace '/' in the file name with '_'
	mname = subst(mname, "/", "_");
	// Replace '.' in the file name with '_'
	mname = subst(mname, ".", "_");
	// Replace ' ' in the file name with '_'
	mname = subst(mname, " ", "_");
	// Replace ':' in the file name with '_'
	mname = subst(mname, ":", "_");
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

	mangledNames[name_] = mname;
	return mname;
}


bool DocFileName::isZipped() const
{
	if (!zipped_valid_) {
		zipped_ = zippedFile(*this);
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
	return lhs.absFilename() == rhs.absFilename() &&
		lhs.saveAbsPath() == rhs.saveAbsPath();
}


bool operator!=(DocFileName const & lhs, DocFileName const & rhs)
{
	return !(lhs == rhs);
}

} // namespace support
} // namespace lyx

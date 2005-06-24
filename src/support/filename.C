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

#include <boost/assert.hpp>

#include <map>
#include <sstream>


using std::map;
using std::string;


namespace lyx {
namespace support {


FileName::FileName()
	: save_abs_path_(true)
{}


FileName::FileName(string const & abs_filename, bool save_abs)
	: name_(abs_filename), save_abs_path_(save_abs)
{
	BOOST_ASSERT(AbsolutePath(name_));
}


void FileName::set(string const & name, string const & buffer_path)
{
	save_abs_path_ = AbsolutePath(name);
	name_ = save_abs_path_ ? name : MakeAbsPath(name, buffer_path);
}


void FileName::erase()
{
	name_.erase();
}


string const FileName::relFilename(string const & path) const
{
	return MakeRelPath(name_, path);
}


string const FileName::outputFilename(string const & path) const
{
	return save_abs_path_ ? name_ : MakeRelPath(name_, path);
}


string const FileName::mangledFilename() const
{
	// We need to make sure that every FileName instance for a given
	// filename returns the same mangled name.
	typedef map<string, string> MangledMap;
	static MangledMap mangledNames;
	MangledMap::const_iterator const it = mangledNames.find(name_);
	if (it != mangledNames.end())
		return (*it).second;

	// Now the real work
	string mname = os::internal_path(name_);
	// Remove the extension.
	mname = ChangeExtension(name_, string());
	// Replace '/' in the file name with '_'
	mname = subst(mname, "/", "_");
	// Replace '.' in the file name with '_'
	mname = subst(mname, ".", "_");
	// Replace ' ' in the file name with '_'
	mname = subst(mname, " ", "_");
	// Add the extension back on
	mname = ChangeExtension(mname, GetExtension(name_));

#if defined(__CYGWIN__) || defined(__CYGWIN32__) || defined(_WIN32)
	// Mangle the drive letter in a Windows-style path.
	if (mname.size() >= 2 && mname[1] == ':')
		mname[1] = '_';
#endif

	// Prepend a counter to the filename. This is necessary to make
	// the mangled name unique.
	static int counter = 0;
	std::ostringstream s;
	s << counter++;
	mname = s.str() + mname;
	mangledNames[name_] = mname;
	return mname;
}


bool FileName::isZipped() const
{
	return zippedFile(name_);
}


string const FileName::unzippedFilename() const
{
	return unzippedFileName(name_);
}


bool operator==(FileName const & lhs, FileName const & rhs)
{
	return lhs.absFilename() == rhs.absFilename() &&
		lhs.saveAbsPath() == rhs.saveAbsPath();
}


bool operator!=(FileName const & lhs, FileName const & rhs)
{
	return !(lhs == rhs);
}

} // namespace support
} // namespace lyx

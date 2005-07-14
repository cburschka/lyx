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


string const FileName::mangledFilename(std::string const & dir) const
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
	// Replace ':' in the file name with '_'
	mname = subst(mname, ":", "_");
	// Add the extension back on
	mname = ChangeExtension(mname, GetExtension(name_));

	// Prepend a counter to the filename. This is necessary to make
	// the mangled name unique.
	static int counter = 0;
	std::ostringstream s;
	s << counter++ << mname;
	mname = s.str();

	// Experiments show that MiKTeX's YAP (version 2.4.1803)
	// will crash if the string referencing the file name in
	// the .dvi file is longer than 220 characters.
	// This string contains about 50 chars-worth of other data,
	// leaving us, say, 160 characters for the file name itself.
	// (Erring on the side of caution.)
	string::size_type max_length = 160;
	if (dir.size() - 1 < max_length) {
		// If dir.size() > max_length, all bets are off anyway.
		// "+ 1" for the directory separator.
		max_length -= dir.size() + 1;

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
	}

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

/**
 * \file filename.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 */

#include <config.h>

#include "filename.h"
#include "support/filetools.h"
#include "lstrings.h"
#include "LAssert.h"


namespace lyx {
namespace support {


FileName::FileName()
	: save_abs_path_(true)
{}


FileName::FileName(string const & abs_filename, bool save_abs)
	: name_(abs_filename), save_abs_path_(save_abs)
{
	Assert(AbsolutePath(name_));
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
	string mname = os::slashify_path(name_);
	// Remove the extension.
	mname = ChangeExtension(name_, string());
	// Replace '/' in the file name with '_'
	mname = subst(mname, "/", "_");
	// Replace '.' in the file name with '_'
	mname = subst(mname, ".", "_");
	// Add the extension back on
	return ChangeExtension(mname, GetExtension(name_));
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

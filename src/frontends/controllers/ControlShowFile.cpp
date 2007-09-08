/**
 * \file ControlShowFile.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlShowFile.h"

#include "support/filetools.h"

using std::string;

namespace lyx {

using support::FileName;
using support::onlyFilename;

namespace frontend {


ControlShowFile::ControlShowFile(Dialog & parent)
	: Controller(parent)
{}


bool ControlShowFile::initialiseParams(string const & data)
{
	filename_ = FileName(data);
	return true;
}


void ControlShowFile::clearParams()
{
	filename_.erase();
}


string ControlShowFile::getFileContents()
{
	return support::getFileContents(filename_);
}


string ControlShowFile::getFileName()
{
	return onlyFilename(filename_.absFilename());
}

} // namespace frontend
} // namespace lyx

/**
 * \file ControlShowFile.C
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

using namespace lyx::support;


ControlShowFile::ControlShowFile(Dialog & parent)
	: Dialog::Controller(parent)
{}


bool ControlShowFile::initialiseParams(string const & data)
{
	filename_ = data;
	return true;
}


void ControlShowFile::clearParams()
{
	filename_.erase();
}


string ControlShowFile::getFileContents()
{
	return GetFileContents(filename_);
}


string ControlShowFile::getFileName()
{
	return OnlyFilename(filename_);
}

/**
 * \file ControlShowFile.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlShowFile.h"

#include "support/filetools.h" // FileSearch


ControlShowFile::ControlShowFile(LyXView & lv, Dialogs & d)
	: ControlDialogBI(lv, d)
{}


void ControlShowFile::showFile(string const & file)
{
	filename_ = file;
	show();
}


string ControlShowFile::getFileContents()
{
	return GetFileContents(filename_);
}


string ControlShowFile::getFileName()
{
	return OnlyFilename(filename_);
}

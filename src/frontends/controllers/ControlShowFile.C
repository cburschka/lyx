/**
 * \file ControlShowFile.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Herbert Voss <voss@perce.de>
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

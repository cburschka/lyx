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
#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "Dialogs.h"
#include "frontends/LyXView.h"
#include "BufferView.h"
#include "gettext.h"

#include "support/filetools.h" // FileSearch

#include <boost/bind.hpp>

#include <fstream>

ControlShowFile::ControlShowFile(LyXView & lv, Dialogs & d)
	: ControlDialogBI(lv, d)
{
	d_.showFile = boost::bind(&ControlShowFile::showFile, this, _1);
}


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

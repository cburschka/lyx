/**
 * \file ControlShowFile.C
 * Copyright 2001 The LyX Team.
 * See the file COPYING.
 *
 * \author Herbert Voss <voss@perce.de>
 */

#include <config.h>
#include <fstream>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "ControlShowFile.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "BufferView.h"
#include "gettext.h"
#include "support/filetools.h" // FileSearch

ControlShowFile::ControlShowFile(LyXView & lv, Dialogs & d)
	: ControlDialog<ControlConnectBI>(lv, d)
{
	d_.showFile.connect(SigC::slot(this, &ControlShowFile::showFile));
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

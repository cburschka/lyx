/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlVCLog.C
 * \author John Levon, moz@compsoc.man.ac.uk
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlVCLog.h"
#include "Lsstream.h"
#include "ViewBase.h"
#include "ButtonControllerBase.h"
#include "buffer.h"
#include "BufferView.h"
#include "Dialogs.h"
#include "lyxrc.h"
#include "gettext.h"

#include "frontends/LyXView.h"

#include "support/lyxlib.h"

#include <boost/bind.hpp>

#include <fstream>

using std::endl;

ControlVCLog::ControlVCLog(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{
	d_.showVCLogFile = boost::bind(&ControlVCLog::show, this);
}

string const ControlVCLog::getBufferFileName() const
{
	return lv_.view()->buffer()->fileName();
}


stringstream & ControlVCLog::getVCLogFile(stringstream & ss) const
{
	string const name = lv_.view()->buffer()->lyxvc.getLogFile();

	std::ifstream in(name.c_str());

	bool found = (in.get());

	if (found) {
		in.seekg(0, std::ios::beg); // rewind to the beginning

		ss << in.rdbuf();
		found = ss.good();
	}

	if (!found)
		ss << _("No version control log file found.") << endl;

	lyx::unlink(name);

	return ss;
}

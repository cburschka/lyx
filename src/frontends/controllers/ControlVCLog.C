/**
 * \file ControlVCLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "ControlVCLog.h"
#include "Lsstream.h"
#include "ButtonControllerBase.h"
#include "buffer.h"
#include "lyxrc.h"
#include "gettext.h"


#include "support/lyxlib.h"

#include <fstream>

using std::endl;


ControlVCLog::ControlVCLog(LyXView & lv, Dialogs & d)
	: ControlDialogBD(lv, d)
{}


string const ControlVCLog::getBufferFileName() const
{
	return buffer()->fileName();
}


stringstream & ControlVCLog::getVCLogFile(stringstream & ss) const
{
	string const name = buffer()->lyxvc.getLogFile();

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

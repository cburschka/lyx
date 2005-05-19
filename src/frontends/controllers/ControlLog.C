/**
 * \file ControlLog.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "ControlLog.h"

#include "gettext.h"
#include "lyxlex.h"

#include <sstream>
#include <fstream>

using std::istringstream;
using std::ostream;
using std::string;

namespace lyx {
namespace frontend {

ControlLog::ControlLog(Dialog & parent)
	: Dialog::Controller(parent),
	  type_(LatexLog)
{}


bool ControlLog::initialiseParams(string const & data)
{
	istringstream is(data);
	LyXLex lex(0,0);
	lex.setStream(is);

	string logtype, logfile;
	lex >> logtype >> logfile;
	if (!lex)
		// Parsing of the data failed.
		return false;

	if (logtype == "latex")
		type_ = LatexLog;
	else if (logtype == "literate")
		type_ = LiterateLog;
	else if (logtype == "lyx2lyx")
		type_ = Lyx2lyxLog;
	else if (logtype == "vc")
		type_ = VCLog;
	else
		return false;

	logfile_ = logfile;
	return true;
}


void ControlLog::clearParams()
{
	logfile_.erase();
}


string const ControlLog::title() const
{
	string t;
	switch (type_) {
	case LatexLog:
		t = _("LaTeX Log");
		break;
	case LiterateLog:
		t = _("Literate Programming Build Log");
		break;
	case Lyx2lyxLog:
		t = _("lyx2lyx Error Log");
		break;
	case VCLog:
		t = _("Version Control Log");
		break;
	}
	return t;
}


void ControlLog::getContents(std::ostream & ss) const
{
	std::ifstream in(logfile_.c_str());

	bool success = false;

	if (in) {
		ss << in.rdbuf();
		success = ss.good();
	}

	if (success)
		return;

	switch (type_) {
	case LatexLog:
		ss << _("No LaTeX log file found.");
		break;
	case LiterateLog:
		ss << _("No literate programming build log file found.");
		break;
	case Lyx2lyxLog:
		ss << _("No lyx2lyx error log file found.");
		break;
	case VCLog:
		ss << _("No version control log file found.");
		break;
	}
}

} // namespace frontend
} // namespace lyx

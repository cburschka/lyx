/**
 * \file Chktex.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Asger Alstrup
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Chktex.h"
#include "LaTeX.h" // TeXErrors
#include "gettext.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxlib.h"
#include "support/systemcall.h"

#include "support/BoostFormat.h"

#include <fstream>

using namespace lyx::support;

using std::ifstream;
using std::getline;


Chktex::Chktex(string const & chktex, string const & f, string const & p)
		: cmd(chktex), file(f), path(p)
{
}


int Chktex::run(TeXErrors &terr)
{
	// run bibtex
	string log = OnlyFilename(ChangeExtension(file, ".log"));
	string tmp = cmd + " -q -v0 -b0 -x " + file + " -o " + log;
	Systemcall one;
	int result = one.startscript(Systemcall::Wait, tmp);
	if (result == 0) {
		result = scanLogFile(terr);
	} else {
		result = -1;
	}
	return result;
}


int Chktex::scanLogFile(TeXErrors & terr)
{
	string token;
	int retval = 0;

	string const tmp = OnlyFilename(ChangeExtension(file, ".log"));

#if USE_BOOST_FORMAT
	boost::format msg(STRCONV(_("ChkTeX warning id # %1$d")));
#else
	string const msg(_("ChkTeX warning id # "));
#endif
	ifstream ifs(tmp.c_str());
	while (getline(ifs, token)) {
		string srcfile;
		string line;
		string pos;
		string warno;
		string warning;
		token = split(token, srcfile, ':');
		token = split(token, line, ':');
		token = split(token, pos, ':');
		token = split(token, warno, ':');
		token = split(token, warning, ':');

		int const lineno = atoi(line);

#if USE_BOOST_FORMAT
		msg % warno;
		terr.insertError(lineno, STRCONV(msg.str()), warning);
		msg.clear();
#else
		terr.insertError(lineno, msg + warno, warning);
#endif

		++retval;
	}
	return retval;
}

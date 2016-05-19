/**
 * \file Chktex.cpp
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

#include "support/convert.h"
#include "support/docstream.h"
#include "support/filetools.h"
#include "support/gettext.h"
#include "support/lstrings.h"
#include "support/Systemcall.h"

using namespace std;
using namespace lyx::support;

namespace lyx {


Chktex::Chktex(string const & chktex, string const & f, string const & p)
		: cmd(chktex), file(f), path(p)
{}


int Chktex::run(TeXErrors &terr)
{
	// run bibtex
	string log = onlyFileName(changeExtension(file, ".log"));
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
	int retval = 0;

	// FIXME: Find out whether onlyFileName() is really needed,
	// or whether makeAbsPath(onlyFileName()) is a noop here
	FileName const tmp(makeAbsPath(onlyFileName(changeExtension(file, ".log"))));

	docstring const msg(_("ChkTeX warning id # %1$s"));
	docstring token;
	// FIXME UNICODE
	// We have no idea what the encoding of the error file is
	ifdocstream ifs(tmp.toFilesystemEncoding().c_str());
	while (getline(ifs, token)) {
		docstring srcfile;
		docstring line;
		docstring pos;
		docstring warno;
		docstring warning;
		token = split(token, srcfile, ':');
		token = split(token, line, ':');
		token = split(token, pos, ':');
		token = split(token, warno, ':');
		token = split(token, warning, ':');

		int const lineno = convert<int>(line);

		terr.insertError(lineno, bformat(msg, warno), warning);

		++retval;
	}
	return retval;
}


} // namespace lyx

/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *          Copyright 2000 The LyX Team.
 *
 * ======================================================
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetcaption.h"
#include "debug.h"

using std::ostream;
using std::endl;

void InsetCaption::Write(Buffer const * buf, ostream & os) const
{
	os << "Caption\n";
	WriteParagraphData(buf, os);
}



void InsetCaption::Read(Buffer const * buf, LyXLex & lex)
{
	string token = lex.GetString();
	if (token != "Caption") {
		lyxerr << "InsetCaption::Read: consistency check failed."
		       << endl;
	}
	InsetText::Read(buf, lex);
}

/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright 1995 Matthias Ettrich
 *          Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetlabel.h"

/* Label. Used to insert a label automatically */


InsetLabel::InsetLabel(string const & cmd)
{
	scanCommand(cmd);
}


InsetLabel * InsetLabel::Clone() const
{
	return new InsetLabel(getCommand());
}


int InsetLabel::GetNumberOfLabels() const
{
	return 1;
}


string InsetLabel::getLabel(int) const
{
	return contents;
}

int InsetLabel::Latex(FILE * file, signed char /*fragile*/)
{
	fprintf(file, "%s", escape(getCommand()).c_str());
	return 0;
}


int InsetLabel::Latex(string & file, signed char /*fragile*/)
{
	file += escape(getCommand());
	return 0;
}


int InsetLabel::Linuxdoc(string & file)
{
	file += "<label id=\"" + getContents() +"\" >";
	return 0;
}


int InsetLabel::DocBook(string & file)
{
	file += "<anchor id=\"" + getContents() +"\" >";
	return 0;
}


// This function escapes 8-bit characters and other problematic characters
// It's exactly the same code as in insetref.C.
string InsetLabel::escape(string const & lab) const {
	char hexdigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
			      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	string enc;
	for (string::size_type i= 0; i < lab.length(); ++i) {
		unsigned char c = lab[i];
		if (c >= 128 || c == '=' || c == '%') {
			enc += '=';
			enc += hexdigit[c >> 4];
			enc += hexdigit[c & 15];
		} else {
			enc += c;
		}
	}
	return enc;
}

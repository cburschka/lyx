/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *          Copyright (C) 1995-1998 The LyX Team.
 *
 *======================================================*/

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "insetlabel.h"

// 	$Id: insetlabel.C,v 1.1 1999/09/27 18:44:39 larsbj Exp $	

#if !defined(lint) && !defined(WITH_WARNINGS)
static char vcid[] = "$Id: insetlabel.C,v 1.1 1999/09/27 18:44:39 larsbj Exp $";
#endif /* lint */

/* Label. Used to insert a label automatically */


InsetLabel::InsetLabel(LString const & cmd)
{
	scanCommand(cmd);
}


InsetLabel::~InsetLabel()
{
}


Inset* InsetLabel::Clone()
{
	InsetLabel *result = new InsetLabel(getCommand());
	return result;
}


int InsetLabel::GetNumberOfLabels() const
{
	return 1;
}


LString InsetLabel::getLabel(int) const
{
	return contents;
}

int InsetLabel::Latex(FILE *file, signed char /*fragile*/)
{
	fprintf(file, "%s", escape(getCommand()).c_str());
	return 0;
}


int InsetLabel::Latex(LString &file, signed char /*fragile*/)
{
	file += escape(getCommand());
	return 0;
}


int InsetLabel::Linuxdoc(LString &file)
{
	file += "<label id=\"" + getContents() +"\" >";
	return 0;
}


int InsetLabel::DocBook(LString &file)
{
	file += "<anchor id=\"" + getContents() +"\" >";
	return 0;
}


// This function escapes 8-bit characters and other problematic characters
// It's exactly the same code as in insetref.C.
LString InsetLabel::escape(LString const & lab) const {
	char hexdigit[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
			      '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	LString enc;
	for (int i=0; i<lab.length(); i++) {
		unsigned char c=lab[i];
		if (c >= 128 || c=='=' || c=='%') {
			enc += '=';
			enc += hexdigit[c>>4];
			enc += hexdigit[c & 15];
		} else {
			enc += (char) c;
		}
	}
	return enc;
}

// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *                       1995-1998 The LyX Team
 *
 *======================================================*/

#ifndef _INSET_LABEL_H
#define _INSET_LABEL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "LString.h"

///
class InsetLabel: public InsetCommand {
public:
	///
	InsetLabel(string const & cmd);
	///
	InsetLabel() : InsetCommand("label") {;}
	///
	~InsetLabel();
	///
	Inset* Clone();
	///
	Inset::Code LyxCode() const { return Inset::LABEL_CODE; }
	///
	int GetNumberOfLabels() const;
	///
	string getLabel(int) const;
	///
	string getScreenLabel() const { return getContents(); }
	///
	unsigned char Editable() const { return 0; }
	///
	int Latex(FILE *file, signed char fragile);
	///
	int Latex(string &file, signed char fragile);
	///
	int Linuxdoc(string &file);
	///
	int DocBook(string &file);
private:
	/// This function escapes 8-bit characters
	string escape(string const &) const;
};

#endif

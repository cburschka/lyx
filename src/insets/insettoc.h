// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Word Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1996-1999 The LyX Team.
 * 
 *======================================================*/

#ifndef INSET_TOC_H
#define INSET_TOC_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "gettext.h"

// Created by Lgb 970527

/** Used to insert table of contents
 */
class InsetTOC: public InsetCommand {
public:
	///
	InsetTOC(): InsetCommand("tableofcontents") {}
	///
	InsetTOC(Buffer * b): InsetCommand("tableofcontents"),owner(b) {}
        ///
        Inset * Clone() { return new InsetTOC(owner); }
    	///
	string getScreenLabel() const { return _("Table of Contents"); }
	/// On edit, we open the TOC pop-up
	void Edit(int, int);
        ///
	unsigned char Editable() const {
		return 1;
	}
	///
	bool Display() const { return true; }
	///
	Inset::Code LyxCode() const { return Inset::TOC_CODE; }
	///
	int Linuxdoc(string & file);
	///
	int DocBook(string & file);
private:
	///
	Buffer * owner;
};

#endif

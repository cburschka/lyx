// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *                        1996-1998 LyX Team
 * 
 *======================================================*/

#ifndef _INSET_LOT_H
#define _INSET_LOT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "gettext.h"

// Created by Lgb 970527

/** Used to insert table of contents
 */
class InsetLOT: public InsetCommand {
public:
	///
	InsetLOT(): InsetCommand("listoftables") {}
	///
	InsetLOT(Buffer *b): InsetCommand("listoftables"), owner(b) {}
        ///
        Inset* Clone() { return new InsetLOT(owner); }
    	///
	string getScreenLabel() const { return _("List of Tables"); }
        
	//void Edit(int, int);
        ///
	unsigned char Editable() const {
		return 0; // not yet
	}
	///
	bool Display() const { return true; }
	///
	Inset::Code LyxCode() const { return Inset::LOT_CODE; }
private:
	///
	Buffer *owner;
};

#endif

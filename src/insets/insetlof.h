// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1996-1999 the LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_LOF_H
#define INSET_LOF_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "gettext.h"

// Created by Lgb 970527

/** Used to insert table of contents
 */
class InsetLOF: public InsetCommand {
public:
	///
	InsetLOF(): InsetCommand("listoffigures") {}
	///
	InsetLOF(Buffer * b): InsetCommand("listoffigures"), owner(b) {}
        ///
        Inset * Clone() { return new InsetLOF(owner); }
    	///
	string getScreenLabel() const { return _("List of Figures"); }
        
	//void Edit(int, int);
        ///
	unsigned char Editable() const {
		return 0; // not yet
	}
	///
	bool Display() const { return true; }
	///
	Inset::Code LyxCode() const { return Inset::LOF_CODE; }
private:
	///
	Buffer * owner;
};
#endif

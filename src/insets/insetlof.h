// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1996-2000 the LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_LOF_H
#define INSET_LOF_H

#include "insetcommand.h"
#include "gettext.h"

// Created by Lgb 970527

#ifdef __GNUG__
#pragma interface
#endif

class Buffer;

/** Used to insert table of contents
 */
class InsetLOF : public InsetCommand {
public:
	///
	InsetLOF() : InsetCommand("listoffigures") {}
	///
	explicit
	InsetLOF(Buffer * b) : InsetCommand("listoffigures"), owner(b) {}
        ///
        Inset * Clone() const { return new InsetLOF(owner); }
    	///
	string getScreenLabel() const;
        
        ///
	EDITABLE Editable() const {
		return NOT_EDITABLE; // not yet
	}
	///
	bool display() const { return true; }
	///
	Inset::Code LyxCode() const { return Inset::LOF_CODE; }
private:
	///
	Buffer * owner;
};
#endif

// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1996-2000 LyX Team
 * 
 * ====================================================== */

#ifndef INSET_LOT_H
#define INSET_LOT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "gettext.h"

// Created by Lgb 970527

class Buffer;

/** Used to insert table of contents
 */
class InsetLOT : public InsetCommand {
public:
	///
	InsetLOT() : InsetCommand("listoftables") {}
	///
	explicit
	InsetLOT(Buffer * b) : InsetCommand("listoftables"), owner(b) {}
        ///
        Inset * Clone() const { return new InsetLOT(owner); }
    	///
	string getScreenLabel() const;
        
        ///
	EDITABLE Editable() const {
		return NOT_EDITABLE; // not yet
	}
	///
	bool display() const { return true; }
	///
	Inset::Code LyxCode() const { return Inset::LOT_CODE; }
private:
	///
	Buffer * owner;
};

#endif

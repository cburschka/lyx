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

#ifndef INSET_LOA_H
#define INSET_LOA_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "gettext.h"

// Created by Bernhard 970807

/** Used to insert table of algorithms
 */
class InsetLOA: public InsetCommand {
public:
	///
	InsetLOA(): InsetCommand("listofalgorithms") {}
	///
	InsetLOA(Buffer * b): InsetCommand("listofalgorithms"), owner(b) {}
        ///
	void Validate(LaTeXFeatures & features) const;
        ///
        Inset * Clone() { return new InsetLOA(owner); }
    	///
	string getScreenLabel() const { return _("List of Algorithms"); }

	
	//void Edit(int, int);
        ///
	unsigned char Editable() const {
		return 0; // not yet
	}
	///
	bool Display() const { return true; }
	///
	Inset::Code LyxCode() const { return Inset::LOA_CODE; }
private:
	///
	Buffer * owner;
};

#endif

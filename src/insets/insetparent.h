// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1997-1998 LyX Team
 * 
 *======================================================*/

#ifndef _INSET_PARENT_H
#define _INSET_PARENT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "gettext.h"
// Created by asierra 970813

/** Reference to the parent document.

  Useful to load a parent document from a child document and to
  share parent's properties between preambleless children. 
 */
class InsetParent: public InsetCommand {
public:
	/// Non-standard LyX macro
	InsetParent(): InsetCommand("lyxparent") { }
	///
        InsetParent(LString fn, Buffer* owner=0);
	///
        ~InsetParent() {}
	/// 
	int Latex(FILE *file, signed char fragile);
	///
	int Latex(LString &file, signed char fragile);
        ///
        Inset* Clone() { return new InsetParent(getContents()); }
    	///
	LString getScreenLabel() const { return LString(_("Parent:"))+getContents(); }
        ///
	void Edit(int, int);
        ///
	unsigned char Editable() const {
		return 1;
	}
        ///
        Inset::Code LyxCode() const { return Inset::PARENT_CODE; }
        ///
        void setParent(LString fn) { setContents(fn); }
};

#endif

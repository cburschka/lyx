// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1997-1999 LyX Team
 * 
 * ====================================================== */

#ifndef INSET_PARENT_H
#define INSET_PARENT_H

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
class InsetParent : public InsetCommand {
public:
	/// Non-standard LyX macro
	InsetParent() : InsetCommand("lyxparent") {}
	///
        InsetParent(string const & fn, Buffer * owner = 0);
	/// 
	int Latex(ostream &, signed char fragile) const;
	///
	int Latex(string & file, signed char fragile) const;
        ///
        Inset * Clone() const { return new InsetParent(getContents()); }
    	///
	string getScreenLabel() const {
		return string(_("Parent:")) + getContents();
	}
        ///
	void Edit(int, int);
        ///
	unsigned char Editable() const {
		return 1;
	}
        ///
        Inset::Code LyxCode() const { return Inset::PARENT_CODE; }
        ///
        void setParent(string fn) { setContents(fn); }
};
#endif

// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1997-2000 LyX Team
 * 
 * ====================================================== */

#ifndef INSET_PARENT_H
#define INSET_PARENT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"
#include "gettext.h"

class Buffer;

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
	int Latex(ostream &, signed char fragile, bool free_spc) const;
        ///
        Inset * Clone() const { return new InsetParent(getContents()); }
    	///
	string getScreenLabel() const {
		return string(_("Parent:")) + getContents();
	}
        ///
	void Edit(BufferView *, int, int, unsigned int);
        ///
	EDITABLE Editable() const {
		return IS_EDITABLE;
	}
        ///
        Inset::Code LyxCode() const { return Inset::PARENT_CODE; }
        ///
        void setParent(string fn) { setContents(fn); }
};
#endif

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

class Buffer;

/** Reference to the parent document.

  Useful to load a parent document from a child document and to
  share parent's properties between preambleless children. 
 */
class InsetParent : public InsetCommand {
public:
	///
	InsetParent(InsetCommandParams const &, Buffer * owner = 0);
	///
	Inset * Clone() const { return new InsetParent(params()); }
    	///
	string getScreenLabel() const;
        ///
	EDITABLE Editable() const { return IS_EDITABLE; }
        ///
        Inset::Code LyxCode() const { return Inset::PARENT_CODE; }
        ///
	void Edit(BufferView *, int, int, unsigned int);
	/// 
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
        ///
        void setParent(string fn) { setContents(fn); }
};
#endif

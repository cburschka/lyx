// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1997-2001 LyX Team
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
	InsetParent(InsetCommandParams const &, Buffer const &, bool same_id = false);
	///
	virtual Inset * clone(Buffer const & buffer, bool same_id = false) const {
		return new InsetParent(params(), buffer, same_id);
	}
    	///
	string const getScreenLabel() const;
        ///
	EDITABLE editable() const { return IS_EDITABLE; }
        ///
        Inset::Code lyxCode() const { return Inset::PARENT_CODE; }
        ///
	void edit(BufferView *, int, int, unsigned int);
	/// 
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
        ///
        void setParent(string fn) { setContents(fn); }
};
#endif

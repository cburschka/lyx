// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 2000-2001 The LyX Team.
 * 
 * ====================================================== */

#ifndef INSET_CITE_H
#define INSET_CITE_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

/** Used to insert citations  
 */
class InsetCitation : public InsetCommand {
public:
	///
	InsetCitation(InsetCommandParams const &);
	///
	Inset * clone(Buffer const &) const {
		return new InsetCitation(params());
	}
	///
	string const getScreenLabel() const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::CITE_CODE; }
        ///
	void edit(BufferView *, int, int, unsigned int);
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
};

#endif // INSET_CITE_H

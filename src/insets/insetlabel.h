// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team
 *
 * ====================================================== */

#ifndef INSET_LABEL_H
#define INSET_LABEL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

class InsetLabel : public InsetCommand {
public:
	///
	InsetLabel(InsetCommandParams const &, bool same_id = false);
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const {
		return new InsetLabel(params(), same_id);
	}
	///
	string const getScreenLabel(Buffer const *) const { return getContents(); }
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset::Code lyxCode() const { return Inset::LABEL_CODE; }
	///
	void edit(BufferView *, int, int, unsigned int);
	///
	void edit(BufferView * bv, bool front = true);
	///
	std::vector<string> const getLabelList() const;
	///
	int latex(Buffer const *, std::ostream &,
	          bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &) const;
};

#endif

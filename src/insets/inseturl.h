// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *	    Copyright 1997-2001 The LyX Team.
 *
 * ====================================================== */

#ifndef INSET_URL_H
#define INSET_URL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcommand.h"

struct LaTeXFeatures;

/** The url inset
 */
class InsetUrl : public InsetCommand {
public:
	///
	explicit
	InsetUrl(InsetCommandParams const &, bool same_id = false);
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const {
		return new InsetUrl(params(), same_id);
	}
	///
	Inset::Code lyxCode() const { return Inset::URL_CODE; }
	///
	void validate(LaTeXFeatures &) const;
	///
	string const getScreenLabel(Buffer const *) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	void edit(BufferView *, int, int, mouse_button::state);
	///
	void edit(BufferView * bv, bool front = true);
	///
	bool display() const { return false; }
	///
	int latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
};

#endif

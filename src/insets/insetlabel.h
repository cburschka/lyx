// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team
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
	InsetLabel(InsetCommandParams const &);
	///
	Inset * Clone(Buffer const &) const { return new InsetLabel(params()); }
	///
	string const getScreenLabel() const { return getContents(); }
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
	///
	Inset::Code LyxCode() const { return Inset::LABEL_CODE; }
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	std::vector<string> const getLabelList() const;
	///
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
private:
	/// This function escapes 8-bit characters
	string const escape(string const &) const;
};

#endif

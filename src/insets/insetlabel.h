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
#include "LString.h"

///
class InsetLabel : public InsetCommand {
public:
	///
	explicit
	InsetLabel(string const & cmd);
	///
	InsetLabel() : InsetCommand("label") {}
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::LABEL_CODE; }
	///
	std::vector<string> getLabelList() const;
	///
	string getScreenLabel() const { return getContents(); }
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
private:
	/// This function escapes 8-bit characters
	string escape(string const &) const;
};

#endif

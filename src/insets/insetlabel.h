// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team
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
	InsetLabel(string const & cmd);
	///
	InsetLabel() : InsetCommand("label") {}
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::LABEL_CODE; }
	///
	int GetNumberOfLabels() const;
	///
	string getLabel(int) const;
	///
	string getScreenLabel() const { return getContents(); }
	///
	EDITABLE Editable() const { return NOT_EDITABLE; }
	///
	int Latex(ostream &, signed char fragile, bool free_spc) const;
#ifndef USE_OSTREAM_ONLY
	///
	int Latex(string & file, signed char fragile, bool free_spc) const;
	///
	int Linuxdoc(string & file) const;
	///
	int DocBook(string & file) const;
#else
	///
	int Linuxdoc(ostream &) const;
	///
	int DocBook(ostream &) const;
#endif
private:
	/// This function escapes 8-bit characters
	string escape(string const &) const;
};

#endif

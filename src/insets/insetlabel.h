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
	unsigned char Editable() const { return 0; }
	///
	int Latex(ostream &, signed char fragile) const;
#ifndef USE_OSTREAM_ONLY
	///
	int Latex(string & file, signed char fragile) const;
#endif
	///
	int Linuxdoc(string & file) const;
	///
	int DocBook(string & file) const;
private:
	/// This function escapes 8-bit characters
	string escape(string const &) const;
};

#endif

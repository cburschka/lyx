// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team. 
 *
 * ====================================================== */

#ifndef INSET_ERROR_H
#define INSET_ERROR_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION
#include "lyxinset.h"
#include "LString.h"
#include "gettext.h"

/** Used for error messages from LaTeX runs.
  
  The edit-operation opens a 
  dialog with the text of the error-message. The inset is displayed as 
  "Error" in a box, and automatically deleted. */
class InsetError: public Inset {
public:
	///
	InsetError(string const & string);
	///
	InsetError();
	///
	~InsetError();
	///
	int ascent(Painter &, LyXFont const & font) const;
	///
	int descent(Painter &, LyXFont const & font) const;
	///
	int width(Painter &, LyXFont const & font) const;
	///
	void draw(Painter &, LyXFont const & font,
		  int baseline, float & x) const;
	///
	void Write(ostream &) const;
	///
	void Read(LyXLex & lex);
	///
	int Latex(ostream &, signed char fragile) const;
	///
	int Latex(string & file, signed char fragile) const;
	///
	int Linuxdoc(string & file) const;
	///
	int DocBook(string & file) const;
	///
	bool AutoDelete() const;
	/// what appears in the minibuffer when opening
	char const * EditMessage() const {return _("Opened error");}
	///
	void Edit(int, int);
	///
	unsigned char Editable() const;
	///
	Inset * Clone() const;
	///
	Inset::Code LyxCode() const { return Inset::NO_CODE; }
	/// We don't want "begin" and "end inset" in lyx-file
	bool DirectWrite() const { return true; };
	///
	static void CloseErrorCB(FL_OBJECT *, long data);
private:
	///
	string contents;
	///
	FL_FORM * form;
	///
	FL_OBJECT * strobj;
};
#endif

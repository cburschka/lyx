// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
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
	int Ascent(LyXFont const & font) const;
	///
	int Descent(LyXFont const & font) const;
	///
	int Width(LyXFont const & font) const;
	///
	void Draw(LyXFont font, LyXScreen & scr, int baseline, float & x);
	///
	void Write(FILE * file);
	///
	void Read(LyXLex & lex);
	///
	int Latex(FILE * file, signed char fragile);
	///
	int Latex(string & file, signed char fragile);
	///
	int Linuxdoc(string & file);
	///
	int DocBook(string & file);
	///
	bool AutoDelete() const;
	/// what appears in the minibuffer when opening
	char const * EditMessage() {return _("Opened error");}
	///
	void Edit(int, int);
	///
	unsigned char Editable() const;
	///
	InsetError * Clone() const;
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

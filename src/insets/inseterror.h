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
	explicit
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
	void draw(BufferView *, LyXFont const & font, int baseline, float & x) const;
	///
	void Write(Buffer const *, std::ostream &) const;
	///
	void Read(Buffer const *, LyXLex & lex);
	///
	int Latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	///
	bool AutoDelete() const;
	/// what appears in the minibuffer when opening
	char const * EditMessage() const;
	///
	void Edit(BufferView *, int x, int y, unsigned int button);
	///
	EDITABLE Editable() const;
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

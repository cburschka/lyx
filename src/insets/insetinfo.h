// -*- C++ -*-
/* This file is part of*
 * ======================================================
 *
 *           LyX, The Document Processor
 * 	 
 *	    Copyright (C) 1995 Matthias Ettrich
 *
 *======================================================*/

#ifndef _INSET_INFO_H
#define _INSET_INFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "lyxinset.h"
#include FORMS_H_LOCATION
#include "LString.h"

/** Info. Handles the note insets.
  
  This class handles the note insets. The edit operation bringes up
  a dialog, where the user can type a note about life or something
  else of interest. When collapsed, the inset is displayed as "Note"
  in a yellow box. Currently, the Read-function is a terrible hack. 
  Some day in the distant future, this will hopefully be obsoleted by 
  a true comment-environment. */
class InsetInfo: public Inset {
public:
	///
	InsetInfo();
	///
	InsetInfo(string const & string);
	///
	~InsetInfo();
	///
	int Ascent(LyXFont const &font) const;
	///
	int Descent(LyXFont const &font) const;
	///
	int Width(LyXFont const &font) const;
	///
	void Draw(LyXFont font, LyXScreen &scr, int baseline, float &x);
	///
	void Write(FILE *file);
	///
	void Read(LyXLex &lex);
	///
	int Latex(FILE *file, signed char fragile);
	///
	int Latex(string &file, signed char fragile);
	///
	int Linuxdoc(string &file);
	///
	int DocBook(string &file);
	/// what appears in the minibuffer when opening
	char const* EditMessage() {return "Opened note";}
	///
	void Edit(int, int);
	///
	unsigned char Editable() const;
	///
	Inset::Code LyxCode() const;
	///
	Inset* Clone();
	///
	static void CloseInfoCB(FL_OBJECT *, long data);
private:
	///
	string contents;
	///
	FL_FORM *form;
	///
	FL_OBJECT *strobj;
};

#endif

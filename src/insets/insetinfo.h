// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 the LyX Team.
 *
 * ====================================================== */

#ifndef INSET_INFO_H
#define INSET_INFO_H

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
class InsetInfo : public Inset {
public:
	///
	InsetInfo();
	///
	InsetInfo(string const & string);
	///
	~InsetInfo();
	///
	int ascent(Painter &, LyXFont const &) const;
	///
	int descent(Painter &, LyXFont const &) const;
	///
	int width(Painter &, LyXFont const &) const;
	///
	void draw(Painter &, LyXFont const &, int baseline, float & x) const;
	///
	void Write(ostream &) const;
	///
	void Read(LyXLex & lex);
	///
	int Latex(ostream &, signed char fragile) const;
#ifndef USE_OSTREAM_ONLY
	///
	int Latex(string & file, signed char fragile) const;
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
	/// what appears in the minibuffer when opening
	const char * EditMessage() const {return _("Opened note");}
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	unsigned char Editable() const;
	///
	Inset::Code LyxCode() const;
	///
	Inset * Clone() const;
	///
	static void CloseInfoCB(FL_OBJECT *, long data);
private:
	///
	string contents;
	///
	FL_FORM * form;
	///
	FL_OBJECT * strobj;
};
#endif

// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 the LyX Team.
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
	explicit
	InsetInfo(string const & string);
	///
	~InsetInfo();
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;
	///
	void Write(Buffer const *, std::ostream &) const;
	///
	void Read(Buffer const *, LyXLex & lex);
	///
	int Latex(Buffer const *, std::ostream &,
		  bool fragile, bool free_spc) const;
	///
	int Ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int Linuxdoc(Buffer const *, std::ostream &) const;
	///
	int DocBook(Buffer const *, std::ostream &) const;
	/// what appears in the minibuffer when opening
	string const EditMessage() const;
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE Editable() const;
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
	///
	LyXFont labelfont;
};
#endif

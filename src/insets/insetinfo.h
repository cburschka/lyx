// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 the LyX Team.
 *
 * ====================================================== */

#ifndef INSET_INFO_H
#define INSET_INFO_H

#ifdef __GNUG__
#pragma interface
#endif

#include "inset.h"
#include FORMS_H_LOCATION
#include "LString.h"
#include "lyxfont.h"

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
	void write(Buffer const *, std::ostream &) const;
	///
	void read(Buffer const *, LyXLex & lex);
	///
	int latex(Buffer const *, std::ostream &,
	          bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docBook(Buffer const *, std::ostream &) const;
	/// what appears in the minibuffer when opening
	string const editMessage() const;
	///
	void edit(BufferView *, int, int, unsigned int);
	///
	void edit(BufferView * bv, bool front = true);
	///
	EDITABLE editable() const;
	///
	Inset::Code lyxCode() const;
	///
	virtual Inset * clone(Buffer const &, bool same_id = false) const;
	///
	static void closeInfoCB(FL_OBJECT *, long data);
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

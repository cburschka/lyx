// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1998 The LyX Team.
 *
 *======================================================
 */
// The pristine updatable inset: Text


#ifndef INSETERT_H
#define INSETERT_H

#ifdef __GNUG__
#pragma interface
#endif

#include "insetcollapsable.h"

/** A collapsable text inset for LaTeX insertions.
  
  To write full ert (including styles and other insets) in a given
  space. 
*/
class InsetERT : public InsetCollapsable {
public:
	///
	InsetERT();
	///
	InsetERT(InsetERT const &, bool same_id = false);
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	InsetERT(string const & contents, bool collapsed);
	///
	Inset::Code lyxCode() const { return Inset::ERT_CODE; }
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	string const editMessage() const;
	///
	bool insertInset(BufferView *, Inset *);
	///
	bool insetAllowed(Inset::Code) const { return false; }
	///
	void setFont(BufferView *, LyXFont const &,
	                     bool toggleall = false, bool selectall = false);
	///
	void edit(BufferView *, int, int, unsigned int);
	///
	void edit(BufferView * bv, bool front = true);
	///
	void insetButtonRelease(BufferView * bv, int x, int y, int button);
	///
	int latex(Buffer const *, std::ostream &, bool fragile,
		  bool free_spc) const;
	///
	int ascii(Buffer const *,
	                  std::ostream &, int linelen = 0) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docBook(Buffer const *, std::ostream &) const;
	///
	UpdatableInset::RESULT localDispatch(BufferView *, kb_action,
	                                     string const &);
	///
	bool checkInsertChar(LyXFont &);
	///
	// this are needed here because of the label/inlined functionallity
	///
	bool needFullRow() const { return !collapsed_ && !inlined(); }
	///
	bool isOpen() const { return !collapsed_ || inlined(); }
	///
	bool inlined() const { return !inset.getAutoBreakRows(); }
	///
	void inlined(BufferView *, bool flag);
	///
	void draw(BufferView *, const LyXFont &, int , float &, bool) const;

private:
	///
	void init();
	///
	string const get_new_label() const;
	///
	void setButtonLabel();
	///
	void set_latex_font(BufferView *);
};

#endif

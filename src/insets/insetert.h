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
#include <sigc++/signal_system.h>

/** A collapsable text inset for LaTeX insertions.
  
  To write full ert (including styles and other insets) in a given
  space.

  Note that collapsed_ encompasses both the inline and collapsed button
  versions of this inset. 
*/

class Language;

class InsetERT : public InsetCollapsable {
public:
	///
	enum ERTStatus {
		Open,
		Collapsed,
		Inlined
	};
	///
	InsetERT(BufferParams const &, bool collapsed = false);
	///
	InsetERT(InsetERT const &, bool same_id = false);
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	InsetERT(BufferParams const &,
		 Language const *, string const & contents, bool collapsed);
	///
	~InsetERT();
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
	EDITABLE editable() const;
	///
	SigC::Signal0<void> hideDialog;
	///
	void insetButtonPress(BufferView *, int x, int y, int button);
	///
	bool insetButtonRelease(BufferView * bv, int x, int y, int button);
	///
	void insetMotionNotify(BufferView *, int x, int y, int state);
	///
	int latex(Buffer const *, std::ostream &, bool fragile,
		  bool free_spc) const;
	///
	int ascii(Buffer const *,
	                  std::ostream &, int linelen = 0) const;
	///
	int linuxdoc(Buffer const *, std::ostream &) const;
	///
	int docbook(Buffer const *, std::ostream &) const;
	///
	void validate(LaTeXFeatures &) const {}
	///
	UpdatableInset::RESULT localDispatch(BufferView *, kb_action,
	                                     string const &);
	///
	bool checkInsertChar(LyXFont &);
	///
	// this are needed here because of the label/inlined functionallity
	///
	bool needFullRow() const { return status_ == Open; }
	///
	bool isOpen() const { return status_ == Open || status_ == Inlined; }
	///
	bool inlined() const { return status_ == Inlined; }
	///
	ERTStatus status() const { return status_; }
	///
	void open(BufferView *);
	///
	void close(BufferView *) const;
	///
	bool allowSpellcheck() { return false; }
	string const selectNextWordToSpellcheck(BufferView *, float &) const;
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, const LyXFont &, int , float &, bool) const;
	/// set the status of the inset
	void status(BufferView *, ERTStatus const st) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	void getDrawFont(LyXFont &) const;
	///
	bool forceDefaultParagraphs(Inset const *) const {
		return true;
	}
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;

private:
	///
	void init();
	///
	string const get_new_label() const;
	///
	void setButtonLabel() const;
	///
	void set_latex_font(BufferView *);
	/// update status on button
	void updateStatus(BufferView *, bool = false) const;

	///
	mutable ERTStatus status_;
};

#endif

// -*- C++ -*-
/* This file is part of*
 * ====================================================== 
 *
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team. 
 *
 * ====================================================== */

#ifndef INSET_ERROR_H
#define INSET_ERROR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "inset.h"
#include "LString.h"
#include <sigc++/signal_system.h>

/** Used for error messages from LaTeX runs.
  
  The edit-operation opens a 
  dialog with the text of the error-message. The inset is displayed as 
  "Error" in a box, and automatically deleted. */
class InsetError : public Inset {
public:
	///
	explicit
	InsetError(string const &);
	///
	~InsetError() { hideDialog(); }
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;
	///
	void write(Buffer const *, std::ostream &) const {}
	///
	void read(Buffer const *, LyXLex &) {}
	///
	int latex(Buffer const *, std::ostream &, bool, bool) const { return 0; }
	///
	int ascii(Buffer const *, std::ostream &, int) const { return 0; }
	///
	int linuxdoc(Buffer const *, std::ostream &) const { return 0; }
	///
	int docBook(Buffer const *, std::ostream &) const { return 0; }
	///
	bool autoDelete() const { return true; }
	/// what appears in the minibuffer when opening
	string const editMessage() const;
	///
	void edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset * clone(Buffer const &) const { return new InsetError(contents); }
	///
	Inset::Code lyxCode() const { return Inset::ERROR_CODE; }
	/// We don't want "begin" and "end inset" in lyx-file
	bool directWrite() const { return true; };
	///
	string const & getContents() const { return contents; }
	///
	SigC::Signal0<void> hideDialog;
private:
	///
	string contents;
};
#endif

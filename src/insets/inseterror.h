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

#include "lyxinset.h"
#include "LString.h"
#include <sigc++/signal_system.h>

#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
#endif

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
	~InsetError() { hide(); }
	///
	int ascent(BufferView *, LyXFont const &) const;
	///
	int descent(BufferView *, LyXFont const &) const;
	///
	int width(BufferView *, LyXFont const &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &, bool) const;
	///
	void Write(Buffer const *, std::ostream &) const {}
	///
	void Read(Buffer const *, LyXLex &) {}
	///
	int Latex(Buffer const *, std::ostream &, bool, bool) const { return 0; }
	///
	int Ascii(Buffer const *, std::ostream &, int) const { return 0; }
	///
	int Linuxdoc(Buffer const *, std::ostream &) const { return 0; }
	///
	int DocBook(Buffer const *, std::ostream &) const { return 0; }
	///
	bool AutoDelete() const { return true; }
	/// what appears in the minibuffer when opening
	string const EditMessage() const;
	///
	void Edit(BufferView *, int, int, unsigned int);
	///
	EDITABLE Editable() const { return IS_EDITABLE; }
	///
	Inset * Clone(Buffer const &) const { return new InsetError(contents); }
	///
	Inset::Code LyxCode() const { return Inset::NO_CODE; }
	/// We don't want "begin" and "end inset" in lyx-file
	bool DirectWrite() const { return true; };
	///
	string const & getContents() const { return contents; }
	///
	Signal0<void> hide;
private:
	///
	string contents;
};
#endif

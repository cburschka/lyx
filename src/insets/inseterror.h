// -*- C++ -*-
/**
 * \file inseterror.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_ERROR_H
#define INSET_ERROR_H

#include "inset.h"
#include "LString.h"

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
	~InsetError();
	///
	virtual dispatch_result localDispatch(FuncRequest const & cmd);
	///
	void dimension(BufferView *, LyXFont const &, Dimension &) const;
	///
	void draw(BufferView *, LyXFont const &, int, float &) const;
	///
	void write(Buffer const *, std::ostream &) const {}
	///
	void read(Buffer const *, LyXLex &) {}
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const { return 0; }
	///
	int ascii(Buffer const *, std::ostream &, int) const { return 0; }
	///
	int linuxdoc(Buffer const *, std::ostream &) const { return 0; }
	///
	int docbook(Buffer const *, std::ostream &, bool) const { return 0; }
	///
	bool autoDelete() const { return true; }
	/// what appears in the minibuffer when opening
	string const editMessage() const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	Inset * clone(Buffer const &) const {
		return new InsetError(contents);
	}
	///
	Inset::Code lyxCode() const { return Inset::ERROR_CODE; }
	/// We don't want "begin" and "end inset" in lyx-file
	bool directWrite() const { return true; };
	///
	string const & getContents() const { return contents; }
private:
	///
	string contents;
};
#endif

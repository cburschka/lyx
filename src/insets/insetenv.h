// -*- C++ -*-
/**
 * \file insetenv.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSETENVIRONMENT_H
#define INSETENVIRONMENT_H

#include "insettext.h"
#include "lyxlayout_ptr_fwd.h"

class InsetEnvironment : public InsetText {
public:
	///
	InsetEnvironment(BufferParams const &, string const & name);
	///
	InsetEnvironment(InsetEnvironment const &);
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	InsetBase * clone() const;
	///
	Inset::Code lyxCode() const { return Inset::ENVIRONMENT_CODE; }
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const;
	///
	string const editMessage() const;
	///
	Inset::EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	bool isTextInset() const { return true; }
	///
	LyXLayout_ptr const & layout() const;
	///
	bool needFullRow() const { return true; }
	/** returns true if, when outputing LaTeX, font changes should
            be closed before generating this inset. This is needed for
            insets that may contain several paragraphs */
	bool noFontChange() const { return true; }

private:
	/// the layout
	LyXLayout_ptr layout_;
};

#endif

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

#include "insetcollapsable.h"


class InsetEnvironment : public InsetCollapsable {
public:
	///
	InsetEnvironment(BufferParams const &, string const & name);
	///
	InsetEnvironment(InsetEnvironment const &, bool same_id = false);
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	Inset::Code lyxCode() const { return Inset::ENVIRONMENT_CODE; }
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool fp) const;
	///
	string const editMessage() const;
	///
	bool needFullRow() const { return true; }
	/** returns true if, when outputing LaTeX, font changes should
            be closed before generating this inset. This is needed for
            insets that may contain several paragraphs */
	bool noFontChange() const { return true; }

private:
	/// LaTeX footer
	string header_;
	/// LaTeX footer
	string footer_;
};

#endif

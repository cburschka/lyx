// -*- C++ -*-
/**
 * \file insetenv.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETENVIRONMENT_H
#define INSETENVIRONMENT_H

#include "insettext.h"
#include "lyxlayout_ptr_fwd.h"


class InsetEnvironment : public InsetText {
public:
	///
	InsetEnvironment(BufferParams const &, std::string const & name);
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	InsetBase::Code lyxCode() const { return InsetBase::ENVIRONMENT_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	std::string const editMessage() const;
	///
	InsetBase::EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	bool isTextInset() const { return true; }
	///
	LyXLayout_ptr const & layout() const;
	/** returns true if, when outputing LaTeX, font changes should
            be closed before generating this inset. This is needed for
            insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
protected:
	InsetEnvironment(InsetEnvironment const &);
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	/// the layout
	LyXLayout_ptr layout_;
};

#endif

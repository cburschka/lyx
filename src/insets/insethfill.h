// -*- C++ -*-
/**
 * \file insethfill.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSET_HFILL_H
#define INSET_HFILL_H


#include "insetcommand.h"

class InsetHFill : public InsetCommand {
public:
	///
	InsetHFill();
	///
	virtual Inset * clone(Buffer const &, bool = false) const {
		return new InsetHFill();
	}
	///
	string const getScreenLabel(Buffer const *) const { return getContents(); }
	///
	Inset::Code lyxCode() const { return Inset::HFILL_CODE; }
	///
	int latex(Buffer const *, std::ostream &, bool fragile, bool free_spc) const;
	///
	int ascii(Buffer const *, std::ostream &, int linelen) const;
	///
	void write(Buffer const * buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }
};

#endif

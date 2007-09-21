// -*- C++ -*-
/**
 * \file InsetHFill.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_HFILL_H
#define INSET_HFILL_H


#include "InsetCommand.h"


namespace lyx {

class InsetHFill : public InsetCommand {
public:
	///
	InsetHFill();
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	docstring const getScreenLabel(Buffer const &) const;
	///
	Inset::Code lyxCode() const { return Inset::HFILL_CODE; }
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	///
	void write(Buffer const & buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }
	/// is this equivalent to a space (which is BTW different from
	// a line separator)?
	bool isSpace() const;
private:
	virtual Inset * clone() const;
};


} // namespace lyx

#endif

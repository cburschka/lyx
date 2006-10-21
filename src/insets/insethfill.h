// -*- C++ -*-
/**
 * \file insethfill.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_HFILL_H
#define INSET_HFILL_H


#include "insetcommand.h"


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
	InsetBase::Code lyxCode() const { return InsetBase::HFILL_CODE; }
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		  OutputParams const & runparams) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const & runparams) const;
	///
	void write(Buffer const & buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }
	/// is this equivalent to a space (which is BTW different from
	// a line separator)?
	bool isSpace() const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const;
};


} // namespace lyx

#endif

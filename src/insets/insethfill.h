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

class InsetHFill : public InsetCommand {
public:
	///
	InsetHFill();
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::HFILL_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const & runparams) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
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

#endif

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
	std::auto_ptr<InsetBase> clone() const;
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::HFILL_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int ascii(Buffer const &, std::ostream &, int linelen) const;
	///
	int linuxdoc(Buffer const &, std::ostream &) const;
	///
	int docbook(Buffer const &, std::ostream &, bool) const;
	///
	void write(Buffer const & buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }

};

#endif

// -*- C++ -*-
/**
 * \file inseturl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_URL_H
#define INSET_URL_H


#include "insetcommand.h"

class LaTeXFeatures;

/** The url inset
 */
class InsetUrl : public InsetCommand {
public:
	///
	explicit
	InsetUrl(InsetCommandParams const &);
	///
	InsetBase::Code lyxCode() const { return InsetBase::URL_CODE; }
	///
	void validate(LaTeXFeatures &) const;
	///
	std::string const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	bool display() const { return false; }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;
private:
	virtual std::auto_ptr<InsetBase> doClone() const {
		return std::auto_ptr<InsetBase>(new InsetUrl(params()));
	}
};

#endif

// -*- C++ -*-
/**
 * \file InsetUrl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_URL_H
#define INSET_URL_H


#include "InsetCommand.h"


namespace lyx {

class LaTeXFeatures;

/** The url inset
 */
class InsetUrl : public InsetCommand {
public:
	///
	explicit
	InsetUrl(InsetCommandParams const &);
	///
	InsetCode lyxCode() const { return URL_CODE; }
	///
	void validate(LaTeXFeatures &) const;
	///
	docstring const getScreenLabel(Buffer const &) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	DisplayType display() const { return Inline; }
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	/// the string that is passed to the TOC
	virtual int textString(Buffer const &, odocstream &,
		OutputParams const &) const;
private:
	virtual Inset * clone() const {
		return new InsetUrl(params());
	}
};


} // namespace lyx

#endif

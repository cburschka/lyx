// -*- C++ -*-
/**
 * \file InsetHyperlink.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_HYPERLINK_H
#define INSET_HYPERLINK_H


#include "InsetCommand.h"


namespace lyx {

class LaTeXFeatures;

/** The hyperlink inset
 */
class InsetHyperlink : public InsetCommand {
public:
	///
	explicit
	InsetHyperlink(InsetCommandParams const &);
	///
	InsetCode lyxCode() const { return HYPERLINK_CODE; }
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
	void textString(Buffer const &, odocstream &) const;
	///
	static CommandInfo const * findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "href"; };
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "href"; }
	/// Force inset into LTR environment if surroundings are RTL?
	virtual bool forceLTR() const { return true; }
private:
	virtual Inset * clone() const {
		return new InsetHyperlink(params());
	}
};


} // namespace lyx

#endif

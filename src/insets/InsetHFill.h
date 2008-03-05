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
	void draw(PainterInfo & pi, int x, int y) const;
	///
	docstring screenLabel() const;
	///
	InsetCode lyxCode() const { return HFILL_CODE; }
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	void write(std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }
	/// is this equivalent to a space (which is BTW different from
	// a line separator)?
	bool isSpace() const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "hfill"; };
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "hfill"; }
private:
	///
	Inset * clone() const { return new InsetHFill(*this); }
};


} // namespace lyx

#endif

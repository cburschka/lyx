// -*- C++ -*-
/**
 * \file InsetFloatList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_FLOATLIST_H
#define INSET_FLOATLIST_H


#include "InsetCommand.h"


namespace lyx {

/** Used to insert table of contents
 */
class InsetFloatList : public InsetCommand {
public:
	///
	InsetFloatList();
	///
	InsetFloatList(std::string const & type);
	///
	docstring screenLabel() const;
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	InsetCode lyxCode() const { return FLOAT_LIST_CODE; }
	///
	DisplayType display() const { return AlignCenter; }
	///
	void write(std::ostream &) const;
	///
	void read(Lexer &);
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const { return 0; }
	///
	int plaintext(odocstream &, OutputParams const & runparams) const;
	///
	void validate(LaTeXFeatures & features) const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "listoftables"; };
	///
	static bool isCompatibleCommand(std::string const & s);
private:
	///
	Inset * clone() const { return new InsetFloatList(*this); }
	///
	static ParamInfo param_info_;
};


} // namespace lyx

#endif

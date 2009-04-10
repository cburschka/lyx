// -*- C++ -*-
/**
 * \file InsetTOC.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_TOC_H
#define INSET_TOC_H

#include "InsetCommand.h"


namespace lyx {


/// Used to insert table of contents and similar lists
class InsetTOC : public InsetCommand {
public:
	///
	explicit InsetTOC(InsetCommandParams const &);
	///
	docstring screenLabel() const;
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	InsetCode lyxCode() const { return TOC_CODE; }
	///
	DisplayType display() const { return AlignCenter; }
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "tableofcontents"; };
	///
	static bool isCompatibleCommand(std::string const & cmd)
		{ return cmd == defaultCommand(); }
private:
	Inset * clone() const { return new InsetTOC(*this); }
};


} // namespace lyx

#endif

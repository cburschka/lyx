// -*- C++ -*-
/**
 * \file InsetIndex.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_INDEX_H
#define INSET_INDEX_H


#include "InsetCollapsable.h"
#include "InsetCommand.h"


namespace lyx {

/** Used to insert index labels
  */
class InsetIndex : public InsetCollapsable {
public:
	///
	InsetIndex(Buffer const &);
private:
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetCode lyxCode() const { return INDEX_CODE; }
	///
	docstring name() const { return from_ascii("Index"); }
	///
	void write(std::ostream & os) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	int latex(odocstream &, OutputParams const &) const;
	/// should paragraph indendation be omitted in any case?
	bool neverIndent() const { return true; }
	///
	void addToToc(ParConstIterator const &) const;
	///
	Inset * clone() const { return new InsetIndex(*this); }
	///
	bool hasFontChanges() const;
};


class InsetPrintIndex : public InsetCommand {
public:
	///
	InsetPrintIndex(InsetCommandParams const &);

	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "printindex"; };
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "printindex"; }
private:
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	InsetCode lyxCode() const;
	///
	DisplayType display() const { return AlignCenter; }
	///
	docstring screenLabel() const;
	///
	Inset * clone() const { return new InsetPrintIndex(*this); }
};


} // namespace lyx

#endif

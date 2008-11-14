// -*- C++ -*-
/**
 * \file InsetNomencl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author O. U. Baran
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_NOMENCL_H
#define INSET_NOMENCL_H


#include "InsetCommand.h"


namespace lyx {

class LaTeXFeatures;

/** Used to insert nomenclature entries
  */
class InsetNomencl : public InsetCommand {
public:
	///
	InsetNomencl(InsetCommandParams const &);
	///
	docstring screenLabel() const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return NOMENCL_CODE; }
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	int docbookGlossary(odocstream &) const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "nomenclature"; };
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "nomenclature"; }
private:
	Inset * clone() const { return new InsetNomencl(*this); }
	/// unique id for this nomenclature entry for docbook export
	docstring nomenclature_entry_id;
};


class InsetPrintNomencl : public InsetCommand {
public:
	///
	InsetPrintNomencl(InsetCommandParams const &);
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	// FIXME: This should be editable to set the label width (stored
	// in params_["labelwidth"]).
	// Currently the width can be read from file and written, but not
	// changed.
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	InsetCode lyxCode() const;
	///
	DisplayType display() const { return AlignCenter; }
	///
	docstring screenLabel() const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "printnomenclature"; };
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "printnomenclature"; }
private:
	Inset * clone() const { return new InsetPrintNomencl(*this); }
};


} // namespace lyx

#endif

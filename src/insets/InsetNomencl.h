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
	InsetNomencl(Buffer * buf, InsetCommandParams const &);

	///
	int docbookGlossary(odocstream &) const;

	/// \name Public functions inherited from Inset class
	//@{
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	bool hasSettings() const { return true; }
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype) const;
	///
	InsetCode lyxCode() const { return NOMENCL_CODE; }
	///
	int docbook(odocstream &, OutputParams const &) const;
	/// Does nothing at the moment.
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "nomenclature"; }
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "nomenclature"; }
	//@}

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const { return new InsetNomencl(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const;
	//@}

	/// unique id for this nomenclature entry for docbook export
	docstring nomenclature_entry_id;
};


class InsetPrintNomencl : public InsetCommand {
public:
	///
	InsetPrintNomencl(Buffer * buf, InsetCommandParams const &);

	/// \name Public functions inherited from Inset class
	//@{
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	/// 
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	InsetCode lyxCode() const;
	///
	bool hasSettings() const { return true; }
	///
	DisplayType display() const { return AlignCenter; }
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	std::string contextMenuName() const;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "printnomenclature"; }
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "printnomenclature"; }
	//@}

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const { return new InsetPrintNomencl(*this); }
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus & status) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	docstring layoutName() const { return from_ascii("PrintNomencl"); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const;
	//@}
};

} // namespace lyx

#endif

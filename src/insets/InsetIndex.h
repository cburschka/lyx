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

class InsetIndexParams {
public:
	///
	explicit InsetIndexParams(docstring const & b = docstring())
		: index(b) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	docstring index;
};


/** Used to insert index labels
  */
class InsetIndex : public InsetCollapsable {
public:
	///
	InsetIndex(Buffer const &, InsetIndexParams const &);
	///
	static std::string params2string(InsetIndexParams const &);
	///
	static void string2params(std::string const &, InsetIndexParams &);
private:
	///
	EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	InsetCode lyxCode() const { return INDEX_CODE; }
	///
	docstring name() const { return from_ascii("Index"); }
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	/// should paragraph indendation be omitted in any case?
	bool neverIndent() const { return true; }
	///
	void addToToc(DocIterator const &);
	///
	docstring const buttonLabel(BufferView const & bv) const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
	///
	Inset * clone() const { return new InsetIndex(*this); }

	///
	friend class InsetIndexParams;
	///
	InsetIndexParams params_;
};


class InsetPrintIndex : public InsetCommand {
public:
	///
	InsetPrintIndex(InsetCommandParams const &);
	///
	InsetCode lyxCode() const { return INDEX_PRINT_CODE; }

	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "printindex"; };
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "printindex"; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	virtual docstring contextMenu(BufferView const & bv, int x, int y) const;
private:
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	DisplayType display() const { return AlignCenter; }
	///
	docstring screenLabel() const;
	///
	Inset * clone() const { return new InsetPrintIndex(*this); }
};


} // namespace lyx

#endif

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
	InsetIndex(Buffer *, InsetIndexParams const &);
	///
	static std::string params2string(InsetIndexParams const &);
	///
	static void string2params(std::string const &, InsetIndexParams &);
private:
	///
	bool hasSettings() const;
	///
	InsetCode lyxCode() const { return INDEX_CODE; }
	///
	docstring layoutName() const { return from_ascii("Index"); }
	///
	ColorCode labelColor() const;
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	/// should paragraph indendation be omitted in any case?
	bool neverIndent() const { return true; }
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype) const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	docstring const buttonLabel(BufferView const & bv) const;
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	std::string contextMenuName() const;
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
	InsetPrintIndex(Buffer * buf, InsetCommandParams const &);

	/// \name Public functions inherited from Inset class
	//@{
	///
	InsetCode lyxCode() const { return INDEX_PRINT_CODE; }
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	std::string contextMenuName() const;
	/// Updates needed features for this inset.
	void validate(LaTeXFeatures & features) const;
	///
	bool hasSettings() const;
	///
	DisplayType display() const { return AlignCenter; }
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "printindex"; }
	///
	static bool isCompatibleCommand(std::string const & s);
	//@}

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const { return new InsetPrintIndex(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const;
	//@}
};


} // namespace lyx

#endif

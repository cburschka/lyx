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
	explicit InsetFloatList(Buffer *);
	///
	InsetFloatList(Buffer *, std::string const & type);

	/// \name Public functions inherited from Inset class
	//@{
	///
	InsetCode lyxCode() const override { return FLOAT_LIST_CODE; }
	///
	int rowFlags() const override { return Display; }
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer &) override;
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override { return; }
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool clickable(BufferView const &, int, int) const override { return true; }
	///
	void validate(LaTeXFeatures & features) const override;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "listoftables"; }
	///
	static bool isCompatibleCommand(std::string const & s);
	//@}

private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const override { return new InsetFloatList(*this); }
	///
	docstring layoutName() const override;
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const override;
	//@}
};


} // namespace lyx

#endif

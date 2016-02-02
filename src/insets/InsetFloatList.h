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
	InsetFloatList(Buffer *);
	///
	InsetFloatList(Buffer *, std::string const & type);

	/// \name Public functions inherited from Inset class
	//@{
	///
	InsetCode lyxCode() const { return FLOAT_LIST_CODE; }
	///
	DisplayType display() const { return AlignCenter; }
	///
	void write(std::ostream &) const;
	///
	void read(Lexer &);
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const { return 0; }
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool clickable(BufferView const &, int, int) const { return true; }
	///
	void validate(LaTeXFeatures & features) const;
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
	Inset * clone() const { return new InsetFloatList(*this); }
	///
	docstring layoutName() const;
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const;
	//@}

	///
	static ParamInfo param_info_;
};


} // namespace lyx

#endif

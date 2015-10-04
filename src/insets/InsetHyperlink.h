// -*- C++ -*-
/**
 * \file InsetHyperlink.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author José Matos
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_HYPERLINK_H
#define INSET_HYPERLINK_H

#include "InsetCommand.h"


namespace lyx {

/** The hyperlink inset
 */
class InsetHyperlink : public InsetCommand
{
public:
	///
	InsetHyperlink(Buffer * buf, InsetCommandParams const &);
				
	/// \name Public functions inherited from Inset class
	//@{
	///
	InsetCode lyxCode() const { return HYPERLINK_CODE; }
	///
	bool hasSettings() const { return true; }
	///
	bool forceLTR() const { return true; }
	///
	bool isInToc() const { return true; }
	///
	void toString(odocstream &) const;
	///
	void forOutliner(docstring &, size_t const, bool const) const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	std::string contextMenuName() const;
	///
	void validate(LaTeXFeatures &) const;
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "href"; }
	///
	static std::string defaultCommand() { return "href"; }
	///
	static ParamInfo const & findInfo(std::string const &);
	//@}
	
private:
	/// \name Private functions inherited from Inset class
	//@{
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const;
	///
	Inset * clone() const { return new InsetHyperlink(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const;
	//@}

	///
	void viewTarget() const;
};


} // namespace lyx

#endif // INSET_HYPERLINK_H

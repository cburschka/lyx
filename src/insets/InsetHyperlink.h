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
	InsetCode lyxCode() const override { return HYPERLINK_CODE; }
	///
	bool hasSettings() const override { return true; }
	///
	bool forceLTR(OutputParams const &) const override { return true; }
	///
	bool isInToc() const override { return true; }
	///
	void toString(odocstream &) const override;
	///
	void forOutliner(docstring &, size_t const, bool const) const override;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	///
	std::string contextMenuName() const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
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
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const override;
	///
	Inset * clone() const override { return new InsetHyperlink(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const override;
	//@}

	///
	void viewTarget() const;
};


} // namespace lyx

#endif // INSET_HYPERLINK_H

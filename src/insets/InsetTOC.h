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

#include "Toc.h"


namespace lyx {

class Paragraph;

/// Used to insert table of contents and similar lists
/// at present, supports only \tableofcontents and \listoflistings.
/// Other such commands, such as \listoffigures, are supported
/// by InsetFloatList.
class InsetTOC : public InsetCommand {
public:
	///
	InsetTOC(Buffer * buf, InsetCommandParams const &);

	/// \name Public functions inherited from Inset class
	//@{
	///
	InsetCode lyxCode() const override { return TOC_CODE; }
	///
	docstring layoutName() const override;
	///
	RowFlags rowFlags() const override { return Display; }
	///
	void validate(LaTeXFeatures &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream & xs, OutputParams const &) const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool clickable(BufferView const &, int, int) const override { return true; }
	//@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "tableofcontents"; }
	///
	static bool isCompatibleCommand(std::string const & cmd);
	//@}

private:
	///
	void makeTOCWithDepth(XMLStream & xs, Toc const & toc, const OutputParams & op) const;
	///
	void makeTOCNoDepth(XMLStream & xs, Toc const & toc, const OutputParams & op) const;
	///
	void makeTOCEntry(XMLStream & xs, Paragraph const & par, OutputParams const & op) const;

	/// \name Private functions inherited from Inset class
	//@{
	///
	Inset * clone() const override { return new InsetTOC(*this); }
	//@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const override;
	//@}
};


} // namespace lyx

#endif

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
	explicit InsetHyperlink(Buffer * buf, InsetCommandParams const &);
	///
	InsetCode lyxCode() const { return HYPERLINK_CODE; }
	///
	void validate(LaTeXFeatures &) const;
	///
	docstring screenLabel() const;
	///
	bool hasSettings() const { return true; }
	///
	DisplayType display() const { return Inline; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	/// the string that is passed to the TOC
	void tocString(odocstream &) const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "href"; }
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "href"; }
	/// Force inset into LTR environment if surroundings are RTL
	bool forceLTR() const { return true; }
	///
	virtual bool isInToc() const { return true; }
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
private:
  	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & flag) const;
	///
	void viewTarget() const;
	///
	Inset * clone() const { return new InsetHyperlink(*this); }
};


} // namespace lyx

#endif

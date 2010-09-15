// -*- C++ -*-
/**
 * \file InsetLine.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 * \author Uwe Stöhr
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LINE_H
#define INSET_LINE_H


#include "InsetCommand.h"


namespace lyx {

class LaTeXFeatures;

class InsetLine : public InsetCommand {
public:
	///
	InsetLine(Buffer * buf, InsetCommandParams const &);
	///
	int docbook(odocstream &, OutputParams const &) const;
	/// Does nothing at the moment.
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	InsetCode lyxCode() const { return LINE_CODE; }
	///
	bool hasSettings() const { return true; }
	///
	docstring screenLabel() const;
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "rule"; };
	///
	static bool isCompatibleCommand(std::string const & s)
		{ return s == "rule"; }
	///
	Dimension const dimension(BufferView const &) const;

private:
	///
	void metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	Inset * clone() const { return new InsetLine(*this); }
};


} // namespace lyx

#endif // INSET_NEWLINE_H

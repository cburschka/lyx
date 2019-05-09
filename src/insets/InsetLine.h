// -*- C++ -*-
/**
 * \file InsetLine.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
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

class InsetLine : public InsetCommand
{
public:
	InsetLine(Buffer * buf, InsetCommandParams const &);

	/// InsetCommand inherited methods.
	//@{
	docstring screenLabel() const;
	static ParamInfo const & findInfo(std::string const &);
	static std::string defaultCommand() { return "rule"; }
	static bool isCompatibleCommand(std::string const & s)
	{ return s == "rule"; }
	//@}

private:

	/// Inset inherited methods.
	//@{
	InsetCode lyxCode() const { return LINE_CODE; }
	int docbook(odocstream &, OutputParams const &) const;
	/// Does nothing at the moment.
	docstring xhtml(XMLStream &, OutputParams const &) const;
	bool hasSettings() const { return true; }
	void metrics(MetricsInfo &, Dimension &) const;
	void draw(PainterInfo & pi, int x, int y) const;
	void latex(otexstream &, OutputParams const &) const;
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	Inset * clone() const { return new InsetLine(*this); }
	//@}

	/// cached line height and offset.
	/// These value are independent of the BufferView size and thus
	/// can be shared between views.
	//@{
	mutable int height_;
	mutable int offset_;
	//@}
};


} // namespace lyx

#endif // INSET_NEWLINE_H

// -*- C++ -*-
/**
 * \file InsetVSpace.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_VSPACE_H
#define INSET_VSPACE_H

#include "Inset.h"
#include "VSpace.h"


namespace lyx {

class InsetVSpace : public Inset
{
public:
	///
	InsetVSpace() : Inset(0) {}
	///
	InsetVSpace(VSpace const &);
	/// How much?
	VSpace const & space() const { return space_; }
	///
	InsetCode lyxCode() const { return VSPACE_CODE; }
	///
	bool hasSettings() const { return true; }
	///
	bool clickable(BufferView const &, int, int) const { return true; }
	///
	std::string contextMenuName() const;
	///
	static void string2params(std::string const &, VSpace &);
	///
	static std::string params2string(VSpace const &);
private:
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void latex(otexstream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	/// Note that this returns the inset rather than writing it,
	/// so it will actually be written after the present paragraph.
	/// The normal case is that this inset will be on a line by
	/// itself, and in that case the present paragraph will not,
	/// in fact, appear at all.
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void read(Lexer & lex);
	///
	void write(std::ostream & os) const;
	///
	DisplayType display() const { return AlignCenter; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	Inset * clone() const { return new InsetVSpace(*this); }
	///
	docstring const label() const;

	///
	VSpace space_;
};


} // namespace lyx

#endif

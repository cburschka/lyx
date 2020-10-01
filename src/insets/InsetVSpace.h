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
	explicit InsetVSpace(VSpace const &);
	/// How much?
	VSpace const & space() const { return space_; }
	///
	InsetCode lyxCode() const override { return VSPACE_CODE; }
	///
	bool hasSettings() const override { return true; }
	///
	bool clickable(BufferView const &, int, int) const override { return true; }
	///
	std::string contextMenuName() const override;
	///
	static void string2params(std::string const &, VSpace &);
	///
	static std::string params2string(VSpace const &);
private:
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	/// Note that this returns the inset rather than writing it,
	/// so it will actually be written after the present paragraph.
	/// The normal case is that this inset will be on a line by
	/// itself, and in that case the present paragraph will not,
	/// in fact, appear at all.
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void read(Lexer & lex) override;
	///
	void write(std::ostream & os) const override;
	///
	RowFlags rowFlags() const override { return Display; }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;
	///
	Inset * clone() const override { return new InsetVSpace(*this); }
	///
	docstring const label() const;

	///
	VSpace space_;
};


} // namespace lyx

#endif

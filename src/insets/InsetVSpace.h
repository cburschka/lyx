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
	InsetVSpace() {}
	///
	InsetVSpace(VSpace const &);
	///
	~InsetVSpace();
	/// How much?
	VSpace const & space() const { return space_; }
	///
	InsetCode lyxCode() const { return VSPACE_CODE; }
	///
	void edit(Cursor & cur, bool front,
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE);
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
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
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
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

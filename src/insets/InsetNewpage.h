// -*- C++ -*-
/**
 * \file InsetNewpage.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_NEWPAGE_H
#define INSET_NEWPAGE_H

#include "Inset.h"


namespace lyx {

class InsetNewpageParams
{
public:
	/// The different kinds of spaces we support
	enum Kind {
		///
		NEWPAGE,
		///
		PAGEBREAK,
		///
		CLEARPAGE,
		///
		CLEARDOUBLEPAGE
	};
	///
	InsetNewpageParams() : kind(NEWPAGE) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Kind kind;
};


class InsetNewpage : public Inset
{
public:
	///
	InsetNewpage();
	///
	explicit InsetNewpage(InsetNewpageParams const & par);
	///
	static void string2params(std::string const &, InsetNewpageParams &);
	///
	static std::string params2string(InsetNewpageParams const &);
private:
	///
	InsetNewpageParams params() const { return params_; }
	///
	InsetCode lyxCode() const { return NEWPAGE_CODE; }
	///
	void metrics(MetricsInfo &, Dimension &) const;
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
	docstring insetLabel() const;
	///
	ColorCode ColorName() const;
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
	///
	Inset * clone() const { return new InsetNewpage(*this); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;

	///
	InsetNewpageParams params_;
};

} // namespace lyx

#endif // INSET_NEWPAGE_H

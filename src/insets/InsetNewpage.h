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
		CLEARDOUBLEPAGE,
		///
		NOPAGEBREAK
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
	InsetCode lyxCode() const override { return NEWPAGE_CODE; }
	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void read(Lexer & lex) override;
	///
	void write(std::ostream & os) const override;
	///
	RowFlags rowFlags() const override { return (params_.kind == InsetNewpageParams::NOPAGEBREAK) ? Inline : Display; }
	///
	docstring insetLabel() const;
	///
	ColorCode ColorName() const;
	///
	std::string contextMenuName() const override;
	///
	Inset * clone() const override { return new InsetNewpage(*this); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;

	///
	InsetNewpageParams params_;
};

} // namespace lyx

#endif // INSET_NEWPAGE_H

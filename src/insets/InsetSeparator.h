// -*- C++ -*-
/**
 * \file InsetSeparator.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Enrico Forestieri
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_SEPARATOR_H
#define INSET_SEPARATOR_H

#include "Inset.h"


namespace lyx {

class InsetSeparatorParams
{
public:
	/// The different kinds of separators we support
	enum Kind {
		PLAIN,
		PARBREAK,
		LATEXPAR
	};
	///
	InsetSeparatorParams() : kind(PLAIN) {}
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	Kind kind;
};


class InsetSeparator : public Inset
{
public:
	///
	InsetSeparator();
	///
	explicit InsetSeparator(InsetSeparatorParams const & par);
	///
	static void string2params(std::string const &, InsetSeparatorParams &);
	///
	static std::string params2string(InsetSeparatorParams const &);
	/// To be used in combination with inset-forall
	/// Here's a command that removes every latexpar separator:
	///   inset-forall Separator:latexpar char-delete-forward
	docstring layoutName() const override
	{
		switch (params_.kind) {
		case InsetSeparatorParams::PLAIN:
			return from_ascii("Separator:plain");
		case InsetSeparatorParams::PARBREAK:
			return from_ascii("Separator:parbreak");
		case InsetSeparatorParams::LATEXPAR:
			return from_ascii("Separator:latexpar");
		}
		// remove warning
		return docstring();
	}
	///
	RowFlags rowFlags() const override { return BreakAfter; }
private:
	///
	InsetCode lyxCode() const override { return SEPARATOR_CODE; }
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
	/// is this equivalent to a space (which is BTW different from
	/// a line separator)?
	bool isSpace() const override { return true; }
	///
	ColorCode ColorName() const;
	///
	std::string contextMenuName() const override;
	///
	Inset * clone() const override { return new InsetSeparator(*this); }
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;

	///
	InsetSeparatorParams params_;
};


} // namespace lyx

#endif // INSET_SEPARATOR_H

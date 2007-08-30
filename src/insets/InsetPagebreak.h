// -*- C++ -*-
/**
 * \file InsetPagebreak.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_PAGEBREAK_H
#define INSET_PAGEBREAK_H


#include "Inset.h"
#include "gettext.h"


namespace lyx {

class InsetPagebreak : public Inset {
public:
	InsetPagebreak() {}

	Inset::Code lyxCode() const { return Inset::PAGEBREAK_CODE; }

	bool metrics(MetricsInfo &, Dimension &) const;

	void draw(PainterInfo & pi, int x, int y) const;

	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;

	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;

	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;

	void read(Buffer const &, Lexer & lex);

	virtual void write(Buffer const & buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }

	DisplayType display() const { return AlignCenter; }

	virtual docstring insetLabel() const { return _("Page Break"); }

	virtual std::string getCmdName() const { return "\\newpage"; }

private:
	virtual Inset * clone() const
	{
		return new InsetPagebreak;
	}
};


class InsetClearPage : public InsetPagebreak {
public:
	InsetClearPage() {}

	docstring insetLabel() const { return _("Clear Page"); }

	std::string getCmdName() const { return "\\clearpage"; }

private:
	virtual Inset * clone() const
	{
		return new InsetClearPage;
	}
};


class InsetClearDoublePage : public InsetPagebreak {
public:
	InsetClearDoublePage() {}

	docstring insetLabel() const { return _("Clear Double Page"); }

	std::string getCmdName() const { return "\\cleardoublepage"; }

private:
	virtual Inset * clone() const
	{
		return new InsetClearDoublePage;
	}
};

} // namespace lyx

#endif // INSET_PAGEBREAK_H

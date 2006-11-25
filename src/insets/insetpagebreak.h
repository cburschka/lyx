// -*- C++ -*-
/**
 * \file insetpagebreak.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_PAGEBREAK_H
#define INSET_PAGEBREAK_H


#include "inset.h"


namespace lyx {

class InsetPagebreak : public InsetOld {
public:
	InsetPagebreak() {}

	InsetBase::Code lyxCode() const { return InsetBase::LINE_CODE; }

	void metrics(MetricsInfo &, Dimension &) const;

	void draw(PainterInfo & pi, int x, int y) const;

	virtual int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;

	int plaintext(Buffer const &, odocstream &,
		  OutputParams const &) const;

	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;

	void read(Buffer const &, LyXLex & lex);

	virtual void write(Buffer const & buf, std::ostream & os) const;
	/// We don't need \begin_inset and \end_inset
	bool directWrite() const { return true; }

	bool display() const { return true; }

	virtual std::string insetLabel() const { return "Page Break"; }

	virtual std::string getCmdName() const { return "\\newpage"; }

private:
	virtual std::auto_ptr<InsetBase> doClone() const
	{
		return std::auto_ptr<InsetBase>(new InsetPagebreak);
	}
};


class InsetClearPage : public InsetPagebreak {
public:
	InsetClearPage() {}

	std::string insetLabel() const { return "Clear Page"; }
	
	std::string getCmdName() const { return "\\clearpage"; }

private:
	virtual std::auto_ptr<InsetBase> doClone() const
	{
		return std::auto_ptr<InsetBase>(new InsetClearPage);
	}
};


class InsetClearDoublePage : public InsetPagebreak {
public:
	InsetClearDoublePage() {}

	std::string insetLabel() const { return "Clear Double Page"; }
	
	std::string getCmdName() const { return "\\cleardoublepage"; }

private:
	virtual std::auto_ptr<InsetBase> doClone() const
	{
		return std::auto_ptr<InsetBase>(new InsetClearDoublePage);
	}
};

} // namespace lyx

#endif // INSET_NEWLINE_H

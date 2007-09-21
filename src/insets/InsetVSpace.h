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
#include "MailInset.h"


namespace lyx {

class InsetVSpace : public Inset {
public:
	///
	InsetVSpace() {}
	///
	InsetVSpace(VSpace const &);
	///
	~InsetVSpace();
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	Dimension const dimension(BufferView const &) const { return dim_; };
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	int latex(Buffer const &, odocstream &,
		  OutputParams const &) const;
	///
	int plaintext(Buffer const &, odocstream &,
		      OutputParams const &) const;
	///
	int docbook(Buffer const &, odocstream &,
		    OutputParams const &) const;
	///
	void read(Buffer const &, Lexer & lex);
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	DisplayType display() const { return AlignCenter; }
	/// How much?
	VSpace const & space() const { return space_; }

protected:
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);

private:
	virtual Inset * clone() const;
	///
	docstring const label() const;

	///
	VSpace space_;
	///
	mutable Dimension dim_;
};


class InsetVSpaceMailer : public MailInset {
public:
	///
	InsetVSpaceMailer(InsetVSpace & inset);
	///
	virtual Inset & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const &, VSpace &);
	///
	static std::string const params2string(VSpace const &);
private:
	///
	static std::string const name_;
	///
	InsetVSpace & inset_;
};


} // namespace lyx

#endif

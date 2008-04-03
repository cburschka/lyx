// -*- C++ -*-
/**
 * \file InsetWrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETWRAP_H
#define INSETWRAP_H

#include "InsetCollapsable.h"
#include "Length.h"


namespace lyx {


class InsetWrapParams {
public:
	///
	void write(std::ostream &) const;
	///
	void read(Lexer &);

	///
	std::string type;
	///
	int lines;
	///
	std::string placement;
	///
	Length overhang;
	///
	Length width;
};


/** The wrap inset
 */
class InsetWrap : public InsetCollapsable {
public:
	///
	InsetWrap(Buffer const &, std::string const &);
	///
	~InsetWrap();
	///
	InsetWrapParams const & params() const { return params_; }
	///
	static void string2params(std::string const &, InsetWrapParams &);
	///
	static std::string params2string(InsetWrapParams const &);
private:
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetCode lyxCode() const { return WRAP_CODE; }
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	int docbook(odocstream &, OutputParams const &) const;
	///
	docstring editMessage() const;
	///
	bool insetAllowed(InsetCode) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	// Update the counters of this inset and of its contents
	void updateLabels(ParIterator const &);
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	docstring name() const;
	///
	Inset * clone() const { return new InsetWrap(*this); }

	///
	InsetWrapParams params_;
};

} // namespace lyx

#endif

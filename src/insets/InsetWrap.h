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

#include "InsetCaptionable.h"

#include "support/Length.h"


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
class InsetWrap : public InsetCaptionable {
public:
	///
	InsetWrap(Buffer *, std::string const &);
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
	void setCaptionType(std::string const & type) override;
	///
	void write(std::ostream & os) const override;
	///
	void read(Lexer & lex) override;
	///
	void validate(LaTeXFeatures & features) const override;
	///
	InsetCode lyxCode() const override { return WRAP_CODE; }
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
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
	bool insetAllowed(InsetCode) const override;
	///
	bool showInsetDialog(BufferView *) const override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	/// Update the label
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	docstring layoutName() const override;
	///
	Inset * clone() const override { return new InsetWrap(*this); }
	/// Is the content of this inset part of the immediate (visible) text sequence?
	bool isPartOfTextSequence() const override { return false; }

	///
	InsetWrapParams params_;
};

} // namespace lyx

#endif

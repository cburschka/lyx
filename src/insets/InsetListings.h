// -*- C++ -*-
/**
 * \file InsetListings.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_LISTINGS_H
#define INSET_LISTINGS_H

#include "InsetListingsParams.h"


namespace lyx {

class LaTeXFeatures;
struct TexString;

/////////////////////////////////////////////////////////////////////////
//
// InsetListings
//
/////////////////////////////////////////////////////////////////////////

/// A captionable and collapsible text inset for program listings.
class InsetListings : public InsetCaptionable
{
public:
	///
	InsetListings(Buffer *, InsetListingsParams const & par = InsetListingsParams());
	///
	~InsetListings();
	///
	static void string2params(std::string const &, InsetListingsParams &);
	///
	static std::string params2string(InsetListingsParams const &);
private:
	///
	bool isLabeled() const override { return true; }
	/// false is needed since listings do their own font handling.
	bool inheritFont() const override { return false; }
	///
	InsetCode lyxCode() const override { return LISTINGS_CODE; }
	/// lstinline is inlined, normal listing is displayed
	int rowFlags() const override;
	///
	docstring layoutName() const override;
	///
	void write(std::ostream & os) const override;
	///
	void read(Lexer & lex) override;
	///
	void latex(otexstream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	void validate(LaTeXFeatures &) const override;
	///
	bool showInsetDialog(BufferView *) const override;
	///
	InsetListingsParams const & params() const { return params_; }
	///
	InsetListingsParams & params() { return params_; }
	///
	std::string contextMenuName() const override;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const override;
	///
	Inset * clone() const override { return new InsetListings(*this); }
	///
	docstring const buttonLabel(BufferView const & bv) const override;
	///
	TexString getCaption(OutputParams const &) const;
	///
	bool insetAllowed(InsetCode c) const override { return c == CAPTION_CODE || c == QUOTE_CODE; }
	///
	Encoding const * forcedEncoding(Encoding const *, Encoding const *) const override;

	///
	InsetListingsParams params_;
};

} // namespace lyx

#endif // INSET_LISTINGS_H

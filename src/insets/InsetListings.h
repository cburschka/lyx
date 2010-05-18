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

#include "LaTeXFeatures.h"
#include "InsetListingsParams.h"


namespace lyx {

/////////////////////////////////////////////////////////////////////////
//
// InsetListings
//
/////////////////////////////////////////////////////////////////////////

/// A collapsable text inset for program listings.
class InsetListings : public InsetCollapsable
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
	bool isLabeled() const { return true; }
	///
	bool noFontChange() const { return true; }
	///
	InsetCode lyxCode() const { return LISTINGS_CODE; }
	/// lstinline is inlined, normal listing is displayed
	DisplayType display() const;
	///
	docstring name() const { return from_ascii("Listings"); }
	// Update the counters of this inset and of its contents
	void updateBuffer(ParIterator const &, UpdateType);
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	int latex(odocstream &, OutputParams const &) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	InsetListingsParams const & params() const { return params_; }
	///
	InsetListingsParams & params() { return params_; }
	///
	docstring contextMenu(BufferView const & bv, int x, int y) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
	///
	Inset * clone() const { return new InsetListings(*this); }
	///
	docstring const buttonLabel(BufferView const & bv) const;
	///
	docstring getCaption(OutputParams const &) const;
	///
	bool insetAllowed(InsetCode c) const { return c == CAPTION_CODE; } 

	///
	InsetListingsParams params_;
};

} // namespace lyx

#endif // INSET_LISTINGS_H

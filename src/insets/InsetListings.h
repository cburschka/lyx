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

#ifndef INSETLISTINGS_H
#define INSETLISTINGS_H

#include "LaTeXFeatures.h"
#include "InsetERT.h"
#include "InsetListingsParams.h"
#include "MailInset.h"


namespace lyx {

/** A collapsable text inset for program listings.
 */


class InsetListings : public InsetERT {
public:
	///
	InsetListings(BufferParams const &, InsetListingsParams const & par = InsetListingsParams());
	///
	~InsetListings();
	///
	Inset::Code lyxCode() const { return Inset::LISTINGS_CODE; }
	/// lstinline is inlined, normal listing is displayed
	virtual DisplayType display() const;
	///
	docstring name() const { return from_ascii("Listings"); }
	// Update the counters of this inset and of its contents
	virtual void updateLabels(Buffer const &, ParIterator const &);
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, Lexer & lex);
	///
	virtual docstring const editMessage() const;
	///
	int latex(Buffer const &, odocstream &, OutputParams const &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	bool metrics(MetricsInfo &, Dimension &) const;
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	bool showInsetDialog(BufferView *) const;
	///
	void getDrawFont(Font &) const;
	///
	InsetListingsParams const & params() const { return params_; }
	///
	InsetListingsParams & params() { return params_; }
protected:
	InsetListings(InsetListings const &);
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	bool getStatus(Cursor & cur, FuncRequest const & cmd, FuncStatus &) const;
private:
	virtual std::auto_ptr<Inset> doClone() const;
	///
	void init();
	///
	void setButtonLabel();
	///
	docstring getCaption(Buffer const &, OutputParams const &) const;
	///
	InsetListingsParams params_;
};


class InsetListingsMailer : public MailInset {
public:
	///
	InsetListingsMailer(InsetListings & inset);
	///
	virtual Inset & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const &,
		InsetListingsParams &);
	///
	static std::string const params2string(InsetListingsParams const &);
private:
	///
	static std::string const name_;
	///
	InsetListings & inset_;
};


} // namespace lyx

#endif

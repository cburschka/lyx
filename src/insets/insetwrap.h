// -*- C++ -*-
/**
 * \file insetwrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef InsetWrap_H
#define InsetWrap_H


#include "insetcollapsable.h"
#include "toc.h"
#include "lyxlength.h"


struct InsetWrapParams {
	///
	void write(std::ostream &) const;
	///
	void read(LyXLex &);

	///
	std::string type;
	///
	std::string placement;
	///
	LyXLength width;
};


/** The wrap inset
 */
class InsetWrap : public InsetCollapsable {
public:
	///
	InsetWrap(BufferParams const &, std::string const &);
	///
	~InsetWrap();
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	void validate(LaTeXFeatures & features) const;
	///
	virtual std::auto_ptr<InsetBase> clone() const;
	///
	InsetOld::Code lyxCode() const { return InsetOld::WRAP_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  LatexRunParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &, bool mixcont) const;
	///
	std::string const editMessage() const;
	///
	bool insetAllowed(InsetOld::Code) const;
	///
	void addToToc(lyx::toc::TocList &, Buffer const &) const;
	///
	bool  showInsetDialog(BufferView *) const;
	///
	int latexTextWidth(BufferView *) const;
	///
	InsetWrapParams const & params() const { return params_; }
protected:
	///
	virtual
	dispatch_result
	priv_dispatch(FuncRequest const & cmd, idx_type &, pos_type &);
private:
	///
	InsetWrapParams params_;
};



#include "mailinset.h"


class InsetWrapMailer : public MailInset {
public:
	///
	InsetWrapMailer(InsetWrap & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const &, InsetWrapParams &);
	///
	static std::string const params2string(InsetWrapParams const &);
private:
	///
	static std::string const name_;
	///
	InsetWrap & inset_;
};

#endif

/**
 * \file insetwrap.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 *
 * Full author contact details are available in file CREDITS
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
	string type;
	///
	string placement;
	///
	LyXLength width;
};


/** The wrap inset
 */
class InsetWrap : public InsetCollapsable {
public:
	///
	InsetWrap(BufferParams const &, string const &);
	///
	InsetWrap(InsetWrap const &, bool same_id = false);
	///
	~InsetWrap();
	///
	virtual dispatch_result localDispatch(FuncRequest const & cmd);	
	///
	void write(Buffer const * buf, std::ostream & os) const;
	///
	void read(Buffer const * buf, LyXLex & lex);
	///
	void validate(LaTeXFeatures & features) const;
	///
	Inset * clone(Buffer const &, bool same_id = false) const;
	///
	Inset::Code lyxCode() const { return Inset::WRAP_CODE; }
	///
	int latex(Buffer const *, std::ostream &,
		  LatexRunParams const &) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
	///
	string const editMessage() const;
	///
	bool insetAllowed(Inset::Code) const;
	///
	int getMaxWidth(BufferView *, UpdatableInset const *) const;
	///
	void addToToc(toc::TocList &, Buffer const *) const;
	///
	bool  showInsetDialog(BufferView *) const;
	///
	int latexTextWidth(BufferView *) const;
	///
	InsetWrapParams const & params() const { return params_; }
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
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string() const;
	///
	static void string2params(string const &, InsetWrapParams &);
	///
	static string const params2string(InsetWrapParams const &);
private:
	///
	static string const name_;
	///
	InsetWrap & inset_;
};

#endif

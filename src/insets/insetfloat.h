// -*- C++ -*-
/**
 * \file insetfloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef INSETFLOAT_H
#define INSETFLOAT_H

#include "insetcollapsable.h"
#include "toc.h"


struct InsetFloatParams {
	///
	InsetFloatParams() : wide(false) {}
	///
	void write(std::ostream & os) const;
	///
	void read(LyXLex & lex);
	///
	string type;
	///
	string placement;
	///
	bool wide;
};


/** The float inset

*/
class InsetFloat : public InsetCollapsable {
public:
	///
	InsetFloat(BufferParams const &, string const &);
	///
	InsetFloat(InsetFloat const &, bool same_id = false);
	///
	~InsetFloat();
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
	Inset::Code lyxCode() const { return Inset::FLOAT_CODE; }
	///
	int latex(Buffer const *, std::ostream &, LatexRunParams const &,
		  bool fp) const;
	///
	int docbook(Buffer const *, std::ostream &, bool mixcont) const;
	///
	string const editMessage() const;
	///
	bool insetAllowed(Inset::Code) const;
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
	///
	void wide(bool w, BufferParams const &);
	///
	void addToToc(toc::TocList &, Buffer const *) const;
	///
	bool  showInsetDialog(BufferView *) const;
	///
	InsetFloatParams const & params() const { return params_; }
	
private:
	///
	InsetFloatParams params_;
};


#include "mailinset.h"


class InsetFloatMailer : public MailInset {
public:
	///
	InsetFloatMailer(InsetFloat & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual string const & name() const { return name_; }
	///
	virtual string const inset2string() const;
	///
	static void string2params(string const &, InsetFloatParams &);
	///
	static string const params2string(InsetFloatParams const &);
private:
	///
	static string const name_;
	///
	InsetFloat & inset_;
};

#endif

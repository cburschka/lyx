// -*- C++ -*-
/**
 * \file insetfloat.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Jürgen Vigna
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETFLOAT_H
#define INSETFLOAT_H

#include "insetcollapsable.h"
#include "toc.h"


class InsetFloatParams {
public:
	///
	InsetFloatParams() : wide(false), sideways(false) {}
	///
	void write(std::ostream & os) const;
	///
	void read(LyXLex & lex);
	///
	std::string type;
	///
	std::string placement;
	///
	bool wide;
	///
	bool sideways;
};


/** The float inset

*/
class InsetFloat : public InsetCollapsable {
public:
	///
	InsetFloat(BufferParams const &, std::string const &);
	///
	~InsetFloat();
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	void validate(LaTeXFeatures & features) const;
	///
	InsetBase::Code lyxCode() const { return InsetBase::FLOAT_CODE; }
	///
	int latex(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;
	///
	std::string const editMessage() const;
	///
	bool insetAllowed(InsetBase::Code) const;
	/** returns true if, when outputing LaTeX, font changes should
	    be closed before generating this inset. This is needed for
	    insets that may contain several paragraphs */
	bool noFontChange() const { return true; }
	///
	void wide(bool w, BufferParams const &);
	///
	void sideways(bool s, BufferParams const &);
	///
	void addToToc(lyx::toc::TocList &, Buffer const &) const;
	///
	bool  showInsetDialog(BufferView *) const;
	///
	InsetFloatParams const & params() const { return params_; }
	///
	bool getStatus(LCursor &, FuncRequest const &, FuncStatus &) const;
protected:
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
private:
	virtual std::auto_ptr<InsetBase> doClone() const;

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
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static void string2params(std::string const &, InsetFloatParams &);
	///
	static std::string const params2string(InsetFloatParams const &);
private:
	///
	static std::string const name_;
	///
	InsetFloat & inset_;
};

#endif

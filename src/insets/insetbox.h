// -*- C++ -*-
/**
 * \file insetbox.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSETBOX_H
#define INSETBOX_H


#include "insetcollapsable.h"
#include "lyxlength.h"


class InsetBoxParams {
public:
	///
	InsetBoxParams(std::string const &);
	///
	void write(std::ostream & os) const;
	///
	void read(LyXLex & lex);
	///
	std::string type;
	/// Use a parbox (true) or minipage (false)
	bool use_parbox;
	/// Do we have an inner parbox or minipage to format paragraphs to
	/// columnwidth?
	bool inner_box;
	///
	LyXLength width;
	/// "special" widths, see usrguide.dvi §3.5
	std::string special;
	///
	char pos;
	///
	char hor_pos;
	///
	char inner_pos;
	///
	LyXLength height;
	///
	std::string height_special;
};


/** The fbox/fancybox inset

*/
class InsetBox : public InsetCollapsable {
public:
	///
	InsetBox(BufferParams const &, std::string const &);
	///
	~InsetBox();
	///
	std::string const editMessage() const;
	///
	InsetBase::Code lyxCode() const { return InsetBase::BOX_CODE; }
	///
	void write(Buffer const &, std::ostream &) const;
	///
	void read(Buffer const & buf, LyXLex & lex);
	///
	void setButtonLabel();
	///
	void metrics(MetricsInfo &, Dimension &) const;
	/// show the Box dialog
	bool showInsetDialog(BufferView * bv) const;
	///
	bool display() const { return false; }
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
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const & runparams) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	InsetBoxParams const & params() const { return params_; }
	///
	enum BoxType {
		Frameless,
		Boxed,
		ovalbox,
		Ovalbox,
		Shadowbox,
		Doublebox
	};
protected:
	InsetBox(InsetBox const &);
	virtual void doDispatch(LCursor & cur, FuncRequest & cmd);
private:
	friend class InsetBoxParams;

	virtual std::auto_ptr<InsetBase> doClone() const;

	/// used by the constructors
	void init();
	///
	InsetBoxParams params_;
};


#include "mailinset.h"

class InsetBoxMailer : public MailInset {
public:
	///
	InsetBoxMailer(InsetBox & inset);
	///
	virtual InsetBase & inset() const { return inset_; }
	///
	virtual std::string const & name() const { return name_; }
	///
	virtual std::string const inset2string(Buffer const &) const;
	///
	static std::string const params2string(InsetBoxParams const &);
	///
	static void string2params(std::string const &, InsetBoxParams &);

private:
	///
	static std::string const name_;
	///
	InsetBox & inset_;
};

#endif // INSET_BOX_H
